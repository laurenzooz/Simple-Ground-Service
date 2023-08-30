#ifndef IMGX_GUI_H
#define IMGX_GUI_H





#include "imgwindow.h"

class SGS_menu : public ImgWindow {

public:
    explicit SGS_menu(ImFontAtlas* fontAtlas = nullptr);
    ~SGS_menu() final = default;

protected:
    void BuildInterface() override;
};




#endif //IMGX_GUI_H