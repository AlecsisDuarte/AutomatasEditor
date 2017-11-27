#include "TextWindow.h"
#include <iostream>
#include <regex>
#include <string>

using namespace Gtk;

TextWindow::TextWindow(BaseObjectType* cobject, const Glib::RefPtr<Builder>& refGlade) : Window(cobject), builder(refGlade){
    color_name = "no_color";
    //We load all our dialogs
    loadBuilderWithFile("Views/aboutDialog.glade", aboutBuilder);
    aboutBuilder->get_widget("aboutDialog", aboutDialog);

    //We load all our Widgets
    refGlade->get_widget("codeView", codeView);
    refGlade->get_widget("errorView", errorView);

    //Give an action to our dialog close button
    Button *dialogCloseButton;
    aboutBuilder->get_widget("closeButton", dialogCloseButton);
    dialogCloseButton->signal_clicked().connect(sigc::mem_fun(*this, &TextWindow::on_close_aboutDialog_clickled));

    //We load every action of our menu bar items
    loadMenuBarActions(refGlade);

    codeBuffer = TextBuffer::create();
    codeBuffer->signal_insert().connect(sigc::mem_fun(*this, &TextWindow::on_insert_event));
    codeView->set_buffer(codeBuffer);

    errorBuffer = TextBuffer::create();
    errorView->set_buffer(errorBuffer);

    create_tags();
    
}

void TextWindow::create_tags(){
    codeBuffer->create_tag("a_color")->property_foreground() = "red";
    codeBuffer->create_tag("e_color")->property_foreground() = "blue";
    codeBuffer->create_tag("i_color")->property_foreground() = "green";
    codeBuffer->create_tag("o_color")->property_foreground() = "purple";
    codeBuffer->create_tag("u_color")->property_foreground() = "orange";
    codeBuffer->create_tag("no_color")->property_foreground() = "black";
}

void TextWindow::changeCharColor(std::string tag_name, Gtk::TextBuffer::iterator &pos, int bytes){
    auto iterEnd = codeBuffer->get_iter_at_offset(pos.get_offset() + bytes);
    codeBuffer->apply_tag_by_name(tag_name, pos, iterEnd);
}

void TextWindow::on_insert_event(const Gtk::TextBuffer::iterator &pos, const Glib::ustring &text, int bytes){
    // std::cout<< "To be inserted in pos" << pos.get_offset() << ": " << text.at(0) << std::endl;
    if (color_name != "no_color" && (text.at(0) == '#' || text.at(0) == 10))
    {
        auto iterStart = codeBuffer->get_iter_at_offset(pos.get_offset() - bytes);
        codeBuffer ->apply_tag_by_name(color_name, iterStart, pos);
        color_name = "no_color";
    }
    else
    {
        if (color_name == "no_color")
        {
            switch (text.at(0))
            {
            case 'A':
                color_name = "a_color";
                break;
            case 'E':
                color_name = "e_color";
                break;
            case 'I':
                color_name = "i_color";
                break;
            case 'O':
                color_name = "o_color";
                break;
            case 'U':
                color_name = "u_color";
                break;
            }
        }
        auto iterStart = codeBuffer->get_iter_at_offset(pos.get_offset() - bytes);
        codeBuffer ->apply_tag_by_name(color_name, iterStart, pos);
    }
}

void TextWindow::on_about_item_clicked(){
    //We open the about dialog
    aboutDialog->show();

    //If the dialog is already open we present it on top of parent
    aboutDialog->present();
}

void TextWindow::on_quit_item_clicked(){
    hide();
}

void TextWindow::on_compile_item_clicked(){
    std::regex correctStartEnd("^(Inicio#)(.|\n)*(Off#)$");
    std::smatch match;
    std::string code = codeBuffer->get_text(false);
    try {
        if(std::regex_search(code, match, correctStartEnd) && match.size() >0){
            errorBuffer->set_text("Inicio y fin correcto");
        } else {
            errorBuffer->set_text("Inicio y fin incorrecto");
        }
    } catch (std::regex_error &e) {
        if (e.code() == std::regex_constants::error_badrepeat)
            std::cerr << "Repeat was not preceded by a valid regular expression.\n";
        else
            std::cerr << "Some other regex exception happened.\n";
    }
}

void TextWindow::loadMenuBarActions(const Glib::RefPtr<Builder> &windowBuilder){
    MenuItem *quitMenuItem, *saveMenuItem, *copyMenuItem, *pasteMenuItem, *compileMenuItem, *runMenuItem, *aboutMenuItem, *manualMenuItem;

    windowBuilder->get_widget("quitItem", quitMenuItem);
    windowBuilder->get_widget("aboutItem", aboutMenuItem);
    windowBuilder->get_widget("compileItem", compileMenuItem);

    quitMenuItem->signal_activate().connect(sigc::mem_fun(*this, &TextWindow::on_quit_item_clicked));
    aboutMenuItem->signal_activate().connect(sigc::mem_fun(*this, &TextWindow::on_about_item_clicked));
    compileMenuItem->signal_activate().connect(sigc::mem_fun(*this, &TextWindow::on_compile_item_clicked));
}

void TextWindow::loadBuilderWithFile(std::string filePath, Glib::RefPtr<Builder> &builder){
    try
    {
        builder = Builder::create_from_file(filePath);
    }
    catch (const Glib::FileError &ex)
    {
        std::cerr << "FileError: " << ex.what() << std::endl;
    }
    catch (const Glib::MarkupError &ex)
    {
        std::cerr << "MarkupError: " << ex.what() << std::endl;
    }
    catch (const Gtk::BuilderError &ex)
    {
        std::cerr << "BuilderError: " << ex.what() << std::endl;
    }
}

void TextWindow::on_close_aboutDialog_clickled(){
    //We close about dialog
    aboutDialog->hide();
}

TextWindow::~TextWindow(){
    aboutDialog = nullptr;
}