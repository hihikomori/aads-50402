#include "application.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace strelnikov
{

  void UndoManager::push(std::unique_ptr< Action > action)
  {
    undoStack_.push_back(std::move(action));
    redoStack_.clear();
  }

  void UndoManager::undo(FileSystem &fs)
  {
    if (undoStack_.empty()) {
      throw std::logic_error("nothing to undo");
    }
    auto action = std::move(undoStack_.back());
    undoStack_.pop_back();
    action->undo(fs);
    redoStack_.push_back(std::move(action));
  }

  void UndoManager::redo(FileSystem &fs)
  {
    if (redoStack_.empty()) {
      throw std::logic_error("nothing to redo");
    }
    auto action = std::move(redoStack_.back());
    redoStack_.pop_back();
    action->execute(fs);
    undoStack_.push_back(std::move(action));
  }

  bool UndoManager::canUndo() const
  {
    return !undoStack_.empty();
  }

  bool UndoManager::canRedo() const
  {
    return !redoStack_.empty();
  }

  Application::Application()
  {
    registerAllActions();
  }

  void Application::run()
  {
    std::string line;

    while (std::getline(std::cin, line)) {
      if (line.empty())
        continue;
      Command cmd = parser_.parse(line);
      if (!cmd.isValid) {
        std::cout << "[ERR] INVALID COMMAND: " << cmd.errorMessage << "\n";
        continue;
      }
      processCommand(cmd, line);
    }
  }

  void Application::processCommand(const Command &cmd, const std::string &rawInput)
  {
    try {
      if (cmd.name == "undo") {
        if (!undoManager_.canUndo()) {
          std::cout << "[ERR] INVALID COMMAND: nothing to undo\n";
        } else {
          undoManager_.undo(fs_);
        }
      } else if (cmd.name == "redo") {
        if (!undoManager_.canRedo()) {
          std::cout << "[ERR] INVALID COMMAND: nothing to redo\n";
        } else {
          undoManager_.redo(fs_);
        }
      } else if (cmd.name == "history") {
        if (commandHistory_.empty()) {
          std::cout << "[OK] NOTHING TO SHOW\n";
        } else {
          for (const auto &h : commandHistory_)
            std::cout << h << "\n";
        }
      } else if (cmd.name == "save") {
        handleSave(cmd.args[0]);
        std::cout << "[OK] SAVED \"" << cmd.args[0] << "\" SESSION\n";
      } else if (cmd.name == "load") {
        handleLoad(cmd.args[0]);
        std::cout << "[OK] LOADED \"" << cmd.args[0] << "\" SESSION\n";
      } else {
        auto action = Action::create(cmd.name, cmd.args);
        action->execute(fs_);
        undoManager_.push(std::move(action));
        commandHistory_.push_back(rawInput);
      }
    } catch (const std::exception &e) {

      std::cout << "[ERR] INVALID COMMAND: " << e.what() << "\n";
    }
  }

  void Application::handleHistory()
  {
    for (const auto &cmdStr : commandHistory_) {
      std::cout << cmdStr << "\n";
    }
  }

  void Application::handleSave(const std::string &filename)
  {
    std::ofstream out(filename);
    if (!out)
      throw std::runtime_error("cannot open file for saving");

    for (const auto &pair : fs_.getTags()) {
      out << "make-tag " << pair.first << "\n";
    }

    for (const auto &pair : fs_.getCollections()) {
      const std::string &name = pair.first;
      const auto &coll = pair.second;
      out << "make-collection " << name << "\n";
      for (const auto &t : coll->getTags()) {
        out << "give-tag " << t << " " << name << "\n";
      }
    }

    for (const auto &pair : fs_.getFiles()) {
      const std::string &path = pair.first;
      const auto &file = pair.second;
      std::string ownerColl = "";
      for (const auto &cPair : fs_.getCollections()) {
        const std::string &cName = cPair.first;
        const auto &coll = cPair.second;
        if (coll->hasFile(path)) {
          ownerColl = cName;
          break;
        }
      }
      if (!ownerColl.empty()) {
        out << "add-file " << ownerColl << " " << path << "\n";

        for (const auto &t : file->getTags()) {
          out << "give-tag " << t << " " << path << "\n";
        }
      }
    }
  }

  void Application::handleLoad(const std::string &filename)
  {
    std::ifstream in(filename);
    if (!in)
      throw std::runtime_error("cannot open file for loading");

    std::string line;
    while (std::getline(in, line)) {
      if (line.empty())
        continue;
      Command cmd = parser_.parse(line);
      if (cmd.isValid) {
        try {
          if (cmd.name == "save" || cmd.name == "load" || cmd.name == "history" || cmd.name == "undo"
              || cmd.name == "redo") {
            continue;
          }
          auto action = Action::create(cmd.name, cmd.args);
          action->execute(fs_);

        } catch (const std::exception &e) {

          std::cout << "[WRN] LOAD FAILED: " << e.what() << " (command: " << line << ")\n";
        }
      }
    }
  }

}