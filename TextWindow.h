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
    std::string filePath;
    std::string fileName;
    bool codeHasError;
    
    Glib::RefPtr<Builder> builder, aboutBuilder, resultBuilder;
    Glib::RefPtr<Gio::SimpleActionGroup> actionGroup;
    Glib::ustring clipboard;
    // std::string lastColorName;

    // // Widgets
    AboutDialog *aboutDialog;
    Dialog *resultDialog;
    Box *vbox;
    TextView *codeView, *errorView, *resultView;
    Glib::RefPtr<TextBuffer> codeBuffer, errorBuffer, resultBuffer;

    //MenuItems
    MenuBar *menuBar;

    //Signals
    void on_about_item_clicked();
    void on_manual_item_clicked();
    void on_quit_item_clicked();
    void on_compile_item_clicked();
    void on_run_item_clicked();
    void on_save_item_clicked();
    void on_open_item_clicked();
    void on_close_aboutDialog_clickled(); //Close button in aboutDialog action
    void on_close_resultDialog_clicked(); //Close button in resultDialog action
    void on_close_manualDialog_clicked(); //Close button in manualDialog action
    void on_insert_event(const Gtk::TextBuffer::iterator &pos, const Glib::ustring &text, int bytes);

    //Copy And Paste Signals
    void on_copy_item_clicked();
    void on_paste_item_clicked();
    void on_clipboard_text_received(const Glib::ustring& text);

    //Methods
    void loadBuilderWithFile(std::string filePath, Glib::RefPtr<Builder> &builder);
    void loadMenuBarActions(const Glib::RefPtr<Builder>& builder);
    void changeCharColor(std::string, Gtk::TextBuffer::iterator &, int);
    void create_tags();
    int saveFileChooser(std::string name, std::string filepath);
    int openFileChooser(std::string filepath);
    bool saveBufferToPath(std::string path, Glib::RefPtr<TextBuffer> &buffer);
    bool openPathToBuffer(std::string path, Glib::RefPtr<TextBuffer> &buffer);

};

#endif //GTKMM_TEXTWINDOW_H