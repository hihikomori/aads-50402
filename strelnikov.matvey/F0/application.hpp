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
    void processCommand(const Command &, const std::string &);

    void handleSave(const std::string &);
    void handleLoad(const std::string &);
    void handleHistory();

    FileSystem fs_;
    Parser parser_;
    UndoManager undoManager_;
    std::vector< std::string > commandHistory_;
  };

}

#endif