#pragma once
#include <JuceHeader.h>

#include "KarpProcessor.h"

//==============================================================================
// MnKarpAudioProcessorEditor: a bar with the rack presets and the size, then the two plugin editors one
// above the other, each under a strip with its on/off switch and a fold arrow, and between them the
// notes going from mnKord to mnArpee. A switched-off stage stays editable but is dimmed.
//
// The editors are the loaded plugins' own (mnKord's and mnArpee's), so the rack is as tall as the two
// together and follows when one of them changes size (mnArpee's resize corner). The two are always
// equally wide. They are windows of their own on the screen, so nothing of the rack can be drawn over
// them: the rack's menus open as windows of their own, and a dialog hides the two while it is open.
//==============================================================================
class MnKarpAudioProcessorEditor : public juce::AudioProcessorEditor,
                                   private juce::Timer
{
public:
    explicit MnKarpAudioProcessorEditor(MnKarpAudioProcessor&);
    ~MnKarpAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    void resized() override;

    // The size of both windows (the zoom of the rack).
    void setZoom(float factor);
    // Sizes the sections and the rack to the editors inside (done by itself when an editor changes).
    void updateSize();

    // For the tests: the size of the plugin editor of a stage on the screen.
    int getStageEditorWidth(MnKarpAudioProcessor::Stage stage) const;
    juce::Point<int> getStageVisualSize(MnKarpAudioProcessor::Stage stage) const;
    juce::AudioProcessorEditor* getStageEditor(MnKarpAudioProcessor::Stage stage) const;
    // The text the note display shows.
    juce::String getFlowText() const;

private:
    // The look of the rack's own controls.
    struct RackLookAndFeel : public juce::LookAndFeel_V4
    {
        RackLookAndFeel();
        juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
        juce::Font getPopupMenuFont() override;
        void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                                  bool highlighted, bool down) override;
        // The menus look like mnKord's and mnArpee's: flat, no border, inside the plugin window.
        void drawPopupMenuBackground(juce::Graphics&, int width, int height) override;
        void drawPopupMenuBackgroundWithOptions(juce::Graphics&, int width, int height,
                                                const juce::PopupMenu::Options&) override;
        void drawPopupMenuItem(juce::Graphics&, const juce::Rectangle<int>& area, bool isSeparator, bool isActive,
                               bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String& text,
                               const juce::String& shortcutKeyText, const juce::Drawable* icon,
                               const juce::Colour* textColour) override;
        juce::Component* getParentComponentForMenuOptions(const juce::PopupMenu::Options&) override;
        int getPopupMenuBorderSize() override { return 0; }
        int getPopupMenuBorderSizeWithOptions(const juce::PopupMenu::Options&) override { return 0; }
        void getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator, int standardMenuItemHeight,
                                       int& idealWidth, int& idealHeight) override;
    };

    // The round on/off switch in a section strip.
    struct PowerButton : public juce::ToggleButton
    {
        PowerButton() { setClickingTogglesState(true); }
        void paintButton(juce::Graphics&, bool highlighted, bool down) override;
    };

    // The fold arrow: down while the editor is shown, right while it is folded.
    struct FoldButton : public juce::Button
    {
        FoldButton() : juce::Button("fold") {}
        void paintButton(juce::Graphics&, bool highlighted, bool down) override;
        bool folded = false;
    };

    // Between the two sections: the notes mnKord gives to mnArpee.
    struct FlowBar : public juce::Component
    {
        explicit FlowBar(MnKarpAudioProcessor& processor) : rack(processor) {}
        void paint(juce::Graphics&) override;
        static juce::String describe(const MnKarpAudioProcessor::NoteBits& notes);

        MnKarpAudioProcessor& rack;
        juce::String shown;
    };

    // A dialog inside the window (as in mnKord and mnArpee): a title, a message, an optional text field and
    // buttons. Enter presses the default button, Escape the cancel button.
    class Dialog : public juce::Component
    {
    public:
        Dialog(const juce::String& title, const juce::String& message, bool withField, const juce::String& initialText,
               const juce::StringArray& buttonNames, int defaultButton, int cancelButton,
               std::function<void(int button, juce::String text)> done);
        void paint(juce::Graphics&) override;
        void resized() override;
        bool keyPressed(const juce::KeyPress&) override;
        void focusFirst();
        // Answers the dialog (once): closes it, then calls `done` with the button and the text.
        void finish(int button);
        int getCancelButton() const { return cancelButton; }

        std::function<void()> onClose;

    private:
        juce::Rectangle<int> cardBounds() const;

        juce::String title, message;
        std::unique_ptr<juce::TextEditor> field;
        juce::OwnedArray<juce::TextButton> buttons;
        int defaultButton, cancelButton;
        std::function<void(int, juce::String)> done;
        bool finished = false;
    };

    // "Drag MIDI": dragged, it hands the take to the DAW as a MIDI file; clicked, it saves it as one.
    struct TakeButton : public juce::Component,
                        public juce::SettableTooltipClient
    {
        explicit TakeButton(MnKarpAudioProcessorEditor& ownerRef) : owner(ownerRef) { setRepaintsOnMouseActivity(true); }
        void paint(juce::Graphics&) override;
        void mouseDrag(const juce::MouseEvent&) override;
        void mouseUp(const juce::MouseEvent&) override;

        MnKarpAudioProcessorEditor& owner;
        bool available = false;
        bool dragged = false;
    };

    // One stage: the strip (switch, name, fold arrow) and the plugin's editor under it.
    class Section : public juce::Component,
                    private juce::ComponentListener
    {
    public:
        // `child` is the stage's plugin, or nullptr when it is not loaded (the section then says why).
        Section(MnKarpAudioProcessorEditor& ownerRef, MnKarpAudioProcessor& processor,
                MnKarpAudioProcessor::Stage stageIn, juce::AudioProcessor* child, const juce::String& role);
        ~Section() override;

        void paint(juce::Graphics&) override;
        void resized() override;

        // The size the section wants: the strip, and the editor's size on the screen.
        int preferredWidth() const;
        int preferredHeight() const;
        juce::Point<int> editorSize() const;
        bool isFolded() const { return rack.isStageFolded(stage); }
        void applyFolded();
        // Asks the plugin to scale its window (the rack's zoom).
        void applyZoom(float factor);
        // The editor's width on the screen (0 without one), and a change of it keeping the height. The plugin may
        // keep another width (its limits): editorWidth() then says what it took.
        int editorWidth() const { return editor != nullptr ? editorSize().x : 0; }
        void setEditorWidth(int width);
        // Hides the plugin's window while a dialog of the rack is open (it would cover the dialog).
        void setEditorHidden(bool shouldBeHidden);
        juce::AudioProcessorEditor* getEditor() const { return editor.get(); }

        static constexpr int stripHeight = 34;

    private:
        void componentMovedOrResized(juce::Component&, bool wasMoved, bool wasResized) override;
        void refreshPower();

        MnKarpAudioProcessorEditor& owner;
        MnKarpAudioProcessor& rack;
        const MnKarpAudioProcessor::Stage stage;
        const juce::String role;

        PowerButton power;
        FoldButton fold;
        juce::Component holder;
        juce::Label missing;   // instead of the editor when the plugin is not loaded
        std::unique_ptr<juce::AudioProcessorEditor> editor;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> powerAttachment;
        bool layingOut = false;
        bool hidden = false;
    };

    void timerCallback() override;
    // Makes the two windows equally wide: the one the user resized wins, else the rack's width.
    void syncWidths();
    void showZoomMenu();

    // Rack presets.
    void showPresetMenu();
    void stepPreset(int delta);
    void saveClicked();
    void promptName(const juce::String& initial);
    void saveAs(const juce::String& name);
    void showSaveFailed();
    void deleteDialog(const juce::String& name);
    // Loads a preset, after asking about the unsaved changes of the current one.
    void loadPresetChecked(const juce::String& name);
    // Calls proceed() now, or after the user saved or discarded the changes of the current rack preset.
    void confirmLeavePreset(std::function<void()> proceed);
    void refreshPresetButton();
    void dragTake();
    void saveTake();
    void showDialog(const juce::String& title, const juce::String& message, bool withField, const juce::String& initialText,
                    const juce::StringArray& buttonNames, int defaultButton, int cancelButton,
                    std::function<void(int, juce::String)> done);

    MnKarpAudioProcessor& rack;
    RackLookAndFeel lookAndFeel;
    juce::TooltipWindow tooltipWindow { this, 500 };

    juce::ArrowButton prevPresetButton { "previous preset", 0.5f, juce::Colour(0xfff0faf3) };
    juce::ArrowButton nextPresetButton { "next preset", 0.0f, juce::Colour(0xfff0faf3) };
    juce::TextButton presetButton, saveButton, zoomButton;
    TakeButton takeButton { *this };
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::Rectangle<int> brandArea;

    std::array<std::unique_ptr<Section>, MnKarpAudioProcessor::numStages> sections;
    FlowBar flowBar;
    juce::Point<int> shownSizes[MnKarpAudioProcessor::numStages];
    bool shownFolded[MnKarpAudioProcessor::numStages] {};
    juce::String shownPreset { "\n" };
    bool shownDirty = false;
    int dirtyTick = 0;
    std::unique_ptr<Dialog> dialog;
    bool syncing = false;
    bool ready = false;   // all sections exist (a section resizing its editor while they are made must not lay out)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MnKarpAudioProcessorEditor)
};
