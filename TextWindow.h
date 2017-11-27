#ifndef GTKMM_TEXTWINDOW_H
#define GTKMM_TEXTWINDOW_H

#include <gtkmm.h>
using namespace Gtk;

class TextWindow : public Window { 
public:
    TextWindow(BaseObjectType* cobject, const Glib::RefPtr<Builder>& refGlade);
    ~TextWindow();

protected:
    // Variables
    std::string color_name;
    
    Glib::RefPtr<Builder> builder, aboutBuilder;
    Glib::RefPtr<Gio::SimpleActionGroup> actionGroup;
    // std::string lastColorName;

    // // Widgets
    AboutDialog *aboutDialog;
    Box *vbox;
    ScrolledWindow *scrollWindow;
    TextView *codeView, *errorView;
    Glib::RefPtr<TextBuffer> codeBuffer, errorBuffer;

    //MenuItems
    Gtk::MenuBar *menuBar;

    //Signals
    void on_about_item_clicked();
    void on_quit_item_clicked();
    void on_compile_item_clicked();
    void on_close_aboutDialog_clickled(); //Close button in aboutDialog action
    void on_insert_event(const Gtk::TextBuffer::iterator &pos, const Glib::ustring &text, int bytes);

    //Methods
    void loadBuilderWithFile(std::string filePath, Glib::RefPtr<Builder> &builder);
    void loadMenuBarActions(const Glib::RefPtr<Builder>& builder);
    void changeCharColor(std::string, Gtk::TextBuffer::iterator &, int);
    void create_tags();
};

#endif //GTKMM_TEXTWINDOW_H