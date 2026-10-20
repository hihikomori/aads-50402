#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <memory>
#include <string>
#include <vector>
#include "actions.hpp"
#include "cli.hpp"
#include "structs.hpp"

namespace strelnikov
{

  class UndoManager
  {
  public:
    void push(std::unique_ptr< Action >);

    void undo(FileSystem &);

    void redo(FileSystem &);

    bool canUndo() const;
    bool canRedo() const;

    void clearRedo();
    void clearUndo();

  private:
    std::vector< std::unique_ptr< Action > > undoStack_;
    std::vector< std::unique_ptr< Action > > redoStack_;
  };

  class Application
  {
  public:
    Application();

    void run();

  private:
    using Handler = void (*)(Application *, const std::string &);
    void processCommand(const Command &, const std::string &);

    static void handleUndo(Application *, const std::string &);
    static void handleRedo(Application *, const std::string &);
    static void handleHistory(Application *, const std::string &);
    static void handleSave(Application *, const std::string &);
    static void handleLoad(Application *, const std::string &);

    void handleSaveLogic(const std::string &);
    void handleLoadLogic(const std::string &);

    FileSystem fs_;
    Parser parser_;
    UndoManager undoManager_;
    std::vector< std::string > commandHistory_;
    std::unordered_map< std::string, Handler > serviceCommands_;
  };

}

#endif