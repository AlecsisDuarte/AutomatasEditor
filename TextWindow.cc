#include "TextWindow.h"
#include <iostream>
#include <regex>
#include <string>
#include <ctime>
#include <cstddef>
#include <fstream>
#define MULTI_LINE_STRING(a) #a
using namespace Gtk;
namespace{
    const char target_text[] = "UTF8_STRING";
}

TextWindow::TextWindow(BaseObjectType* cobject, const Glib::RefPtr<Builder>& refGlade) : Window(cobject), builder(refGlade){
    codeHasError = true;
    color_name = "no_color";
    filePath.clear();
    fileName = "untitled.txt";
    set_title(fileName + " - TextIDE");

    //We load all our dialogs from our files
    loadBuilderWithFile("Views/aboutDialog.glade", aboutBuilder);
    loadBuilderWithFile("Views/resultDialog.glade", resultBuilder);

    //We create our Dialog widgets
    aboutBuilder->get_widget("aboutDialog", aboutDialog);
    aboutDialog->set_transient_for(*this);
    resultBuilder->get_widget("resultDialog", resultDialog);
    resultDialog->set_transient_for(*this);
    
    //We load all our Widgets
    refGlade->get_widget("codeView", codeView);
    refGlade->get_widget("errorView", errorView);
    resultBuilder->get_widget("resultView", resultView);

    //Give an action to our dialog close button
    Button *aboutCloseButton, *resultCloseButton;
    aboutBuilder->get_widget("closeButton", aboutCloseButton);
    resultBuilder->get_widget("closeButton", resultCloseButton);

    aboutCloseButton->signal_clicked().connect(sigc::mem_fun(*this, &TextWindow::on_close_aboutDialog_clickled));
    resultCloseButton->signal_clicked().connect(sigc::mem_fun(*this, &TextWindow::on_close_resultDialog_clicked));

    //We load every action of our menu bar items
    loadMenuBarActions(refGlade);

    codeBuffer = TextBuffer::create();
    codeBuffer->signal_insert().connect(sigc::mem_fun(*this, &TextWindow::on_insert_event));
    codeView->set_buffer(codeBuffer);

    errorBuffer = TextBuffer::create();
    errorView->set_buffer(errorBuffer);

    resultBuffer = TextBuffer::create();
    resultView->set_buffer(resultBuffer);

    create_tags();
    
}
void TextWindow::create_tags() {
    codeBuffer->create_tag("a_color")->property_foreground() = "red";
    codeBuffer->create_tag("e_color")->property_foreground() = "blue";
    codeBuffer->create_tag("i_color")->property_foreground() = "green";
    codeBuffer->create_tag("o_color")->property_foreground() = "purple";
    codeBuffer->create_tag("u_color")->property_foreground() = "orange";
    codeBuffer->create_tag("no_color")->property_foreground() = "black";

    errorBuffer->create_tag("error_color")->property_foreground() = "red";
    errorBuffer->create_tag("success_color")->property_foreground() = "green";
}

void TextWindow::changeCharColor(std::string tag_name, Gtk::TextBuffer::iterator &pos, int bytes){
    auto iterEnd = codeBuffer->get_iter_at_offset(pos.get_offset() + bytes);
    codeBuffer->apply_tag_by_name(tag_name, pos, iterEnd);
}

void TextWindow::on_insert_event(const Gtk::TextBuffer::iterator &pos, const Glib::ustring &text, int bytes){
    set_title("*"+fileName + " - TextIDE");
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

void TextWindow::on_manual_item_clicked(){
    MessageDialog dialog(*this, "TextIDE little manual");
    const char *message =
        "In order to use this editor, you must understand the sintax.\n"
        "The first thing you have to do is to start with an Incio # and\n"
        "end with an Off #, inside of this two methods you can write\n"
        "two different methods like ENSAJE which enables you to\n"
        "write to the result dialog and the UNDO method is used\n"
        "to repeat the times the ENSAJE message is going to be\n"
        "appear inside the dialog. One Example being:\n\n"
        "Inicio #\n"
        "Undo [5]#\n"
        "Ensaje \"Hola\"#\n"
        "Off #";
    dialog.set_secondary_text(message);
    dialog.run();
}

void TextWindow::on_quit_item_clicked(){
    hide();
}

void TextWindow::on_compile_item_clicked(){
    std::clock_t begin = clock();
    //We start our error buffer
    errorBuffer->set_text("");

    //We get the beginning of our buffer
    TextBuffer::iterator iter = errorBuffer->get_iter_at_offset(0);
    std::ostringstream ostr;

    //We create our regexes
    std::regex correctStartEnd("^(\\s)*(Inicio#)(.|\\s)*(\\s)(Off#)(\\s)*$");
    std::regex hasUndo("\\b(Undo)(.)*");
    std::regex hasEnsaje("\\bEnsaje(.)*");
    // std::regex hasUnknownMethod("(Inicio#)(Ensaje(.)*|Undo(.)*|\\s)+(Off#)");
    std::regex hasUnknownMethod("(Inicio#)(Ensaje(.*)#|Undo(.*)#|\\s)+(Off#)");

    std::smatch match;
    std::string code(codeBuffer->get_text(false));
    try {
        //Verify if we have correct Start and End of code
        if(!std::regex_match(code, correctStartEnd)){
            ostr << "\tError at the Start and/or end of the code\n";
        }else {
            if (!std::regex_match(code, hasUnknownMethod)) {
                ostr << "\tHas Unknown or unclosed(#) methods\n";
            }
        }
        //Verify if we have an Undo Method
        if (std::regex_search(code, hasUndo)){
            // std::cout << "Code has UNDO method" << std::endl;
            std::string codePart = code;
            bool hasError = false;
            std::regex correctUndo("\\bUndo (\\u005B)(\\d+)(\\u005D)#");
            while (std::regex_search(codePart, match, hasUndo) && !hasError){
                if (!std::regex_search(match[0].str(), correctUndo)){
                    ostr << "\tUndo(s) has sintaxes error\n";
                    hasError = true;
                }
                codePart = match.suffix().str();
                //We verify if undo has an Ensaje after it
                if(!std::regex_search(codePart, std::regex("^(\\s| )*\\bEnsaje"))){
                    ostr<< "\tUndo must be acompanied by an Ensaje" << std::endl;
                    hasError = true;
                }
            }
        }
        
        // //Verify if we have Ensaje Method
        if(std::regex_search(code, match, hasEnsaje)){
            // std::cout << "Code has ENSAJE method" << std::endl;
            std::regex correctEnsaje("\\bEnsaje (\\u0022)([0-z]|\\s)+(\\u0022)#");
            std::string codePart = code;
            bool hasError = false;
            while(std::regex_search(codePart, match, hasEnsaje) && !hasError){
                if(!std::regex_search(match[0].str(), correctEnsaje)){
                    ostr << "\tEnsaje(s) has sintaxes error\n";
                    hasError = true;
                }
                codePart = match.suffix().str();
            }
        }
    } catch (std::regex_error &e) {
        if (e.code() == std::regex_constants::error_badrepeat)
            std::cerr << "Repeat was not preceded by a valid regular expression.\n";
        else
            std::cerr << "Some other regex exception happened.\n";
    }
    clock_t end = clock();
    float runtime_secs = (float(end) - float(begin)) / CLOCKS_PER_SEC;

    //If we didn't collect any error we show a success message
    if(ostr.str().size() == 0){
        // std::cout << "Compiled succesfully!" << std::endl;
        ostr << "Successfully compiled\n";
        ostr << "\tCompilation time: " << runtime_secs << " segs\n";
        errorBuffer->insert_with_tag(iter, ostr.str(), "success_color");
        codeHasError = false;
    } else {
        Glib::ustring errors = "Compilation Errors:\n" + ostr.str();
        std::cout << "Compiled with errors!" << std::endl;
        errorBuffer->insert_with_tag(iter, errors, "error_color");
        codeHasError = true;
    }

}

void TextWindow::on_run_item_clicked(){
    on_compile_item_clicked();
    if(!codeHasError){
        //We initialize our resultBuffer
        resultBuffer->set_text("");
        TextBuffer::iterator iter = resultBuffer->get_iter_at_offset(0);

        std::ostringstream ostr;
        std::regex hasUndo("\\b(Undo)(.)*");
        std::regex getNumber("\\d+");
        std::regex hasEnsaje("\\bEnsaje(.)*");
        std::regex getMessage("\\u0022([0-z]|\\s)+\\u0022");
        std::smatch match;
        std::string code(codeBuffer->get_text(false));
        //We add an EOL in order to get every single method including the Off#
        code.append("\n");

        while (std::regex_search(code, match, std::regex("([0-z]| |\\u0022)+#\\s")))
        {   
            long times = 1;
            std::string codeLine = match[0].str();
            std::smatch tmpMatch;
            //We start looking if the code line is an Undo
            if(std::regex_search(codeLine, hasUndo)){
                //We get our number from the Undo string
                std::regex_search(codeLine, tmpMatch, getNumber);
                times = std::atoi(tmpMatch[0].str().c_str());

                //We go to our next Line
                code = match.suffix().str();
                std::regex_search(code, match, std::regex("([0-z]| |\\u0022)+#\\s"));
            } 
            //In case the line changed passing throug the hasUndo part we get the new line
            codeLine = match[0].str();
            if(std::regex_search(codeLine, tmpMatch, getMessage)){
                while(times--){
                    //we remove the Quoatation Mark
                    std::string message = tmpMatch[0].str();
                    ostr << message.substr(1, message.size()-2) << "\n";
                }
            }
            code = match.suffix().str();            
        }
        resultBuffer->insert(iter, ostr.str());
        resultDialog->show();
        resultDialog->present();
    }
}

void TextWindow::on_save_item_clicked(){
    if(filePath.empty()){
        saveFileChooser("untitled.txt", ".");
        set_title(fileName + "- TextIDE");
    } else {
        std::cout << "You have specified the filename("<< filePath << ")" << std::endl;
        saveBufferToPath(filePath, codeBuffer);
    }

}

void TextWindow::on_open_item_clicked(){
    openFileChooser(filePath.empty()? ".": filePath);
}

void TextWindow::on_copy_item_clicked(){
    std::cout << "You tried to copy" << std::endl;
    Glib::RefPtr<Clipboard> refClipboard = Clipboard::get();
    if(errorBuffer->get_has_selection()){
        errorBuffer->copy_clipboard(refClipboard);
    } else{
        codeBuffer->copy_clipboard(refClipboard);
    }
}

void TextWindow::on_paste_item_clicked(){
    std::cout << "You tried to paste" << std::endl;
    Glib::RefPtr<Clipboard> refClipboard = Clipboard::get();

    refClipboard->request_text(sigc::mem_fun(*this, &TextWindow::on_clipboard_text_received));

}

void TextWindow::on_clipboard_text_received(const Glib::ustring& text){
    codeBuffer->insert_at_cursor(text);
}

void TextWindow::loadMenuBarActions(const Glib::RefPtr<Builder> &windowBuilder){
    MenuItem *quitMenuItem, *saveMenuItem, *compileMenuItem, *runMenuItem, *aboutMenuItem, *manualMenuItem, *openMenuItem, *copyMenuItem, *pasteMenuItem;

    windowBuilder->get_widget("quitItem", quitMenuItem);
    windowBuilder->get_widget("aboutItem", aboutMenuItem);
    windowBuilder->get_widget("compileItem", compileMenuItem);
    windowBuilder->get_widget("runItem", runMenuItem);
    windowBuilder->get_widget("saveItem", saveMenuItem);
    windowBuilder->get_widget("openItem", openMenuItem);
    windowBuilder->get_widget("manualItem", manualMenuItem);
    windowBuilder->get_widget("copyItem", copyMenuItem);
    windowBuilder->get_widget("pasteItem", pasteMenuItem);

    quitMenuItem->signal_activate().connect(sigc::mem_fun(*this, &TextWindow::on_quit_item_clicked));
    aboutMenuItem->signal_activate().connect(sigc::mem_fun(*this, &TextWindow::on_about_item_clicked));
    compileMenuItem->signal_activate().connect(sigc::mem_fun(*this, &TextWindow::on_compile_item_clicked));
    runMenuItem->signal_activate().connect(sigc::mem_fun(*this, &TextWindow::on_run_item_clicked));
    saveMenuItem->signal_activate().connect(sigc::mem_fun(*this, &TextWindow::on_save_item_clicked));
    openMenuItem->signal_activate().connect(sigc::mem_fun(*this, &TextWindow::on_open_item_clicked));
    manualMenuItem->signal_activate().connect(sigc::mem_fun(*this, &TextWindow::on_manual_item_clicked));
    copyMenuItem->signal_activate().connect(sigc::mem_fun(*this, &TextWindow::on_copy_item_clicked));
    pasteMenuItem->signal_activate().connect(sigc::mem_fun(*this, &TextWindow::on_paste_item_clicked));
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

int TextWindow::saveFileChooser(std::string name, std::string path){
    FileChooserDialog dialog("Please choose a folder", FILE_CHOOSER_ACTION_SAVE);
    dialog.set_transient_for(*this);

    dialog.add_button("_Cancel", RESPONSE_CANCEL);
    dialog.add_button("_Save", RESPONSE_OK);

    auto filter_text = FileFilter::create();
    filter_text->set_name("Text files");
    filter_text->add_mime_type("text/plain");
    dialog.add_filter(filter_text);
    dialog.set_current_name(name);
    dialog.set_current_folder(path);

    int result = dialog.run();
    switch (result) {
        case (RESPONSE_OK): {
            filePath = dialog.get_filename();
            fileName = dialog.get_current_name();
            saveBufferToPath(filePath, codeBuffer);
            set_title(fileName + "- TextIDE");
            break;
        }
        case (RESPONSE_CANCEL): {
            std::cout << "Saving canceled" << std::endl;
            break;
        }
        default: {
            std::cout << "Unexpected button clicked." << std::endl;
            break;
        }
    }
    return result;
}

int TextWindow::openFileChooser(std::string path){
    FileChooserDialog dialog("Please choose a file", FILE_CHOOSER_ACTION_OPEN);
    dialog.set_transient_for(*this);

    dialog.add_button("_Cancel", RESPONSE_CANCEL);
    dialog.add_button("_Open", RESPONSE_OK);

    auto filer_text = FileFilter::create();
    filer_text->set_name("Text files");
    filer_text->add_mime_type("text/plain");
    dialog.add_filter(filer_text);

    int result = dialog.run();
    switch(result){
        case(RESPONSE_CANCEL):{
            std::cout << "Cancel file opening" << std::endl;
            break;
        }
        case(RESPONSE_OK):{
            std::cout << "File opened: " << dialog.get_filename() << std::endl;
            filePath = dialog.get_filename();
            std::smatch match;
            std::regex_search(filePath, match, std::regex("([A-z]+).txt"));
            fileName = match[0].str();
            std::cout << "File name: " << fileName << std::endl;
            if(openPathToBuffer(filePath, codeBuffer)){
                set_title(fileName + " - TextIDE"); 
            }
            break;
        }
        default: {
            std::cout << "Unexpected button clicked." << std::endl;
        }
    }
    return result;
}

bool TextWindow::saveBufferToPath(std::string path, Glib::RefPtr<TextBuffer> &buffer){
    std::ifstream f(path, std::ifstream::in);
    if(f.good()){
        std::ofstream file(filePath, std::ofstream::trunc);
        file << buffer->get_text(false);
        set_title(fileName + " - TextIDE");
    }else {
        std::ofstream file(filePath, std::ofstream::out);
        file << buffer->get_text(false);
    }
    return true;
}

bool TextWindow::openPathToBuffer(std::string path, Glib::RefPtr<TextBuffer> &buffer){
    std::ifstream file(path, std::ifstream::in);
    if(!file){
        std::cerr << "Couldn't open file at: " << path << std::endl;
        return false;
    }
    std::string line;
    codeBuffer->set_text("");
    TextBuffer::iterator iter = codeBuffer->get_iter_at_offset(0);
    // buffer->signal_insert().disconnect();
    while(std::getline(file, line)){
        std::string color = "no_color";
            switch (line.at(0))
            {
            case 'A':
                color = "a_color";
                break;
            case 'E':
                color = "e_color";
                break;
            case 'I':
                color = "i_color";
                break;
            case 'O':
                color = "o_color";
                break;
            case 'U':
                color = "u_color";
                break;
            }
        iter = codeBuffer->insert_with_tag(iter, line.append("\n"), color);
        }
    return true;
}

void TextWindow::on_close_aboutDialog_clickled()
{
    //We close about dialog
    aboutDialog->hide();
}

void TextWindow::on_close_resultDialog_clicked(){
    //We close about dialog
    resultDialog->hide();
}

TextWindow::~TextWindow(){
    aboutDialog = nullptr;
}