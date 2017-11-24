#ifndef GTKMM_TEXTWINDOW_H
#define GTKMM_TEXTWINDOW_H

#include <gtkmm.h>
using namespace Gtk;

class TextWindow : public Window { 
public:
    
    TextWindow(BaseObjectType* cobject, const Glib::RefPtr<Builder>& refGlade);

protected:
    Glib::RefPtr<Builder> builder;
    // std::string lastColorName;

    // // Widgets
    // Window* window;
    // Box *m_vbox;
    // ScrolledWindow *m_scrollWindow;
    // TextView *m_textView;
    // Glib::RefPtr<TextBuffer> m_textBuffer;
};

#endif //GTKMM_TEXTWINDOW_H