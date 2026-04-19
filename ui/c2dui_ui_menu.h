//
// Created by cpasjuste on 30/01/18.
//

#ifndef C2DUI_UI_MENU_NEW_H
#define C2DUI_UI_MENU_NEW_H

namespace c2dui {
    class UIHighlight;
    class MenuLine;

    class UiMenu : public SkinnedRectangle {

    public:

        explicit UiMenu(UiMain *uiMain);

        ~UiMenu() override;

        virtual void load(bool isRomMenu = false);

        UiMain *getUi() { return ui; };

        bool isRom() const { return isRomMenu; };

        virtual bool isOptionHidden(Option *option) { return false; };

        void onKeyUp(bool animate = true);

        void onKeyDown(bool animate = true);

        bool onInput(c2d::Input::Player *players) override;

        void setVisibility(c2d::Visibility visibility, bool tweenPlay = false) override;

    protected:

        void updateLines();

        void moveSelection(int direction, bool animate);

        void resetContentAnimation();

        void startContentScrollAnimation(float fromY, int durationMs);

        UiMain *ui = nullptr;
        SkinnedText *title = nullptr;
        c2d::RectangleShape *content = nullptr;
        c2d::TweenPosition *contentTween = nullptr;
        UIHighlight *highlight = nullptr;
        std::vector<MenuLine *> lines;
        c2d::Vector2f contentBasePos = {0, 0};
        int pendingCursorAnimMs = 0;
        float alpha = 230;

        std::vector<Option> options;
        c2d::TweenPosition *tweenPosition;
        Skin::TextGroup textGroup;

        float lineHeight = 0;
        int maxLines = 0;
        int optionIndex = 0;
        int highlightIndex = 0;
        const int cursorAnimMs = 100;
        const int contentAnimMs = 100;
        const int valueSlideAnimMs = 100;

        bool isRomMenu = false;
        bool isEmuRunning = false;
        bool needSave = false;
    };
}

#endif //C2DUI_UI_MENU_NEW_H
