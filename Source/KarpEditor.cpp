#include "KarpEditor.h"

#include <BinaryData.h>
#include <mnKord/MnAudioUi.h>

namespace
{
using mnaudio::ui::Palette;

constexpr int margin = 8;
constexpr int gap = 8;
constexpr int barHeight = 40;
constexpr int flowHeight = 24;
constexpr int minContentWidth = 640;

juce::Font titleFont(float height)
{
    static const juce::Typeface::Ptr montserrat = juce::Typeface::createSystemTypefaceFor(
        BinaryData::Montserrat_ttf, (size_t) BinaryData::Montserrat_ttfSize);
    return juce::Font(juce::FontOptions(montserrat).withHeight(height)).boldened();
}

juce::Font textFont(float height)
{
    static const juce::Typeface::Ptr roboto = juce::Typeface::createSystemTypefaceFor(
        BinaryData::RobotoCondensed_ttf, (size_t) BinaryData::RobotoCondensed_ttfSize);
    return juce::Font(juce::FontOptions(roboto).withHeight(height)).boldened();
}

juce::String noteName(int note)
{
    static const char* const names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    return juce::String(names[note % 12]) + juce::String(note / 12 - 1);
}
}

//==============================================================================
MnKarpAudioProcessorEditor::RackLookAndFeel::RackLookAndFeel()
{
    setColour(juce::TextButton::buttonColourId, Palette::cardHi());
    setColour(juce::TextButton::textColourOffId, Palette::text());
    setColour(juce::TextButton::textColourOnId, Palette::text());
    setColour(juce::PopupMenu::backgroundColourId, Palette::card());
    setColour(juce::PopupMenu::textColourId, Palette::text());
    setColour(juce::PopupMenu::highlightedBackgroundColourId, Palette::cardHi());
    setColour(juce::PopupMenu::highlightedTextColourId, Palette::accent());
    setColour(juce::AlertWindow::backgroundColourId, Palette::card());
    setColour(juce::AlertWindow::textColourId, Palette::text());
    setColour(juce::TextEditor::backgroundColourId, Palette::bg());
    setColour(juce::TextEditor::textColourId, Palette::text());
    setColour(juce::TextEditor::outlineColourId, Palette::border());
}

juce::Font MnKarpAudioProcessorEditor::RackLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight)
{
    return textFont(juce::jmin(16.0f, (float) buttonHeight * 0.6f));
}

juce::Font MnKarpAudioProcessorEditor::RackLookAndFeel::getPopupMenuFont()
{
    return textFont(14.0f);
}

void MnKarpAudioProcessorEditor::RackLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                                                       const juce::Colour& backgroundColour,
                                                                       bool highlighted, bool down)
{
    auto fill = backgroundColour;
    if (down)
        fill = fill.brighter(0.16f);
    else if (highlighted)
        fill = fill.brighter(0.08f);
    if (! button.isEnabled())
        fill = fill.withMultipliedAlpha(0.5f);

    g.setColour(fill);
    g.fillRoundedRectangle(button.getLocalBounds().toFloat(), 7.0f);
}

void MnKarpAudioProcessorEditor::RackLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    g.setColour(findColour(juce::PopupMenu::backgroundColourId));
    g.fillRect(0, 0, width, height);
}

void MnKarpAudioProcessorEditor::RackLookAndFeel::drawPopupMenuBackgroundWithOptions(
    juce::Graphics& g, int width, int height, const juce::PopupMenu::Options&)
{
    drawPopupMenuBackground(g, width, height);
}

void MnKarpAudioProcessorEditor::RackLookAndFeel::drawPopupMenuItem(
    juce::Graphics& g, const juce::Rectangle<int>& area, bool isSeparator, bool isActive, bool isHighlighted,
    bool isTicked, bool, const juce::String& text, const juce::String&, const juce::Drawable*,
    const juce::Colour* textColour)
{
    if (isSeparator)
    {
        g.setColour(findColour(juce::PopupMenu::textColourId).withAlpha(0.2f));
        g.fillRect(area.withSizeKeepingCentre(area.getWidth() - 16, 1));
        return;
    }

    auto row = area.reduced(0, 1);
    if (isHighlighted)
    {
        g.setColour(findColour(juce::PopupMenu::highlightedBackgroundColourId));
        g.fillRect(row);
    }

    auto colour = textColour != nullptr ? *textColour
                                        : findColour(isHighlighted ? juce::PopupMenu::highlightedTextColourId
                                                                   : juce::PopupMenu::textColourId);
    if (! isActive)
        colour = colour.withAlpha(0.45f);

    g.setColour(colour);
    g.setFont(textFont(14.0f));
    g.drawText(text, row.reduced(28, 0), juce::Justification::centredLeft, true);

    if (isTicked)
    {
        juce::Path tick;
        const auto tickArea = row.removeFromLeft(24).toFloat().reduced(7.0f, 8.0f);
        tick.startNewSubPath(tickArea.getX(), tickArea.getCentreY());
        tick.lineTo(tickArea.getCentreX() - 1.0f, tickArea.getBottom());
        tick.lineTo(tickArea.getRight(), tickArea.getY());
        g.strokePath(tick, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
}

juce::Component* MnKarpAudioProcessorEditor::RackLookAndFeel::getParentComponentForMenuOptions(
    const juce::PopupMenu::Options& options)
{
    if (auto* parent = options.getParentComponent())
        return parent;

    if (auto* target = options.getTargetComponent())
        return target->findParentComponentOfClass<juce::AudioProcessorEditor>();

    return nullptr;
}

void MnKarpAudioProcessorEditor::RackLookAndFeel::getIdealPopupMenuItemSize(
    const juce::String& text, bool isSeparator, int standardMenuItemHeight, int& idealWidth, int& idealHeight)
{
    idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight : 24;

    if (isSeparator)
    {
        idealWidth = 50;
        idealHeight = idealHeight / 2;
        return;
    }

    // The text area of drawPopupMenuItem() has 28 px at each side, plus a little room.
    idealWidth = juce::GlyphArrangement::getStringWidthInt(textFont(14.0f), text) + 56 + 16;
}

//==============================================================================
void MnKarpAudioProcessorEditor::PowerButton::paintButton(juce::Graphics& g, bool highlighted, bool down)
{
    const auto r = getLocalBounds().toFloat().reduced(1.0f);
    const bool on = getToggleState();
    const auto line = on ? Palette::accent() : Palette::dimText().withAlpha(0.55f);

    g.setColour(on ? Palette::accent().withAlpha(down ? 0.34f : highlighted ? 0.26f : 0.18f)
                   : (down ? Palette::cardHi().brighter(0.2f) : highlighted ? Palette::cardHi().brighter(0.1f)
                                                                           : Palette::cardHi()));
    g.fillEllipse(r);
    g.setColour(line);
    g.drawEllipse(r, 1.5f);

    // The power symbol: an arc open at the top, with a bar in the gap.
    const auto c = r.getCentre();
    const float radius = r.getWidth() * 0.26f;
    juce::Path symbol;
    symbol.addCentredArc(c.x, c.y, radius, radius, 0.0f, 0.75f, juce::MathConstants<float>::twoPi - 0.75f, true);
    symbol.startNewSubPath(c.x, c.y - radius * 1.2f);
    symbol.lineTo(c.x, c.y - radius * 0.1f);
    g.strokePath(symbol, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void MnKarpAudioProcessorEditor::FoldButton::paintButton(juce::Graphics& g, bool highlighted, bool down)
{
    const auto r = getLocalBounds().toFloat().reduced(1.0f);
    g.setColour(down ? Palette::cardHi().brighter(0.2f) : highlighted ? Palette::cardHi().brighter(0.1f) : Palette::cardHi());
    g.fillRoundedRectangle(r, 5.0f);
    g.setColour(Palette::border());
    g.drawRoundedRectangle(r, 5.0f, 1.0f);

    const auto c = r.getCentre();
    juce::Path arrow;
    if (folded)
        arrow.addTriangle(c.x - 3.0f, c.y - 5.0f, c.x - 3.0f, c.y + 5.0f, c.x + 4.0f, c.y);
    else
        arrow.addTriangle(c.x - 5.0f, c.y - 3.0f, c.x + 5.0f, c.y - 3.0f, c.x, c.y + 4.0f);
    g.setColour(Palette::text());
    g.fillPath(arrow);
}

void MnKarpAudioProcessorEditor::Dim::paint(juce::Graphics& g)
{
    g.fillAll(Palette::bg().withAlpha(0.55f));
}

//==============================================================================
juce::String MnKarpAudioProcessorEditor::FlowBar::describe(const MnKarpAudioProcessor::NoteBits& notes)
{
    juce::StringArray names;
    for (int note = 0; note < 128; ++note)
        if (notes.contains(note))
            names.add(noteName(note));

    if (names.isEmpty())
        return {};

    constexpr int maxShown = 10;
    const int extra = names.size() - maxShown;
    if (extra > 0)
    {
        names.removeRange(maxShown, extra);
        return names.joinIntoString("  ") + "  +" + juce::String(extra);
    }
    return names.joinIntoString("  ");
}

void MnKarpAudioProcessorEditor::FlowBar::paint(juce::Graphics& g)
{
    shown = describe(rack.getNotesAfter(MnKarpAudioProcessor::kordStage));

    auto area = getLocalBounds();
    const auto arrow = area.removeFromLeft(34).toFloat();
    const auto c = arrow.getCentre();
    juce::Path head;
    head.addTriangle(c.x - 5.0f, c.y - 2.0f, c.x + 5.0f, c.y - 2.0f, c.x, c.y + 5.0f);
    g.setColour(shown.isEmpty() ? Palette::border() : Palette::accent());
    g.fillPath(head);
    g.drawLine(c.x, c.y - 8.0f, c.x, c.y - 2.0f, 2.0f);

    g.setFont(textFont(14.0f));
    g.setColour(Palette::dimText().withAlpha(0.7f));
    g.drawText("MIDI from mnKord to mnArpee", area.removeFromLeft(190), juce::Justification::centredLeft);

    g.setColour(Palette::accent());
    g.setFont(textFont(15.0f));
    g.drawText(shown.isEmpty() ? juce::String("-") : shown, area, juce::Justification::centredLeft);
}

//==============================================================================
MnKarpAudioProcessorEditor::Section::Section(MnKarpAudioProcessorEditor& ownerRef, MnKarpAudioProcessor& processor,
                                             MnKarpAudioProcessor::Stage stageIn, juce::AudioProcessor& child,
                                             const juce::String& roleIn)
    : owner(ownerRef), rack(processor), stage(stageIn), role(roleIn)
{
    const juce::String name = MnKarpAudioProcessor::stageName(stage);
    power.setTooltip("Switch " + name + " off or on. Off: the MIDI passes it untouched.");
    fold.setTooltip("Show or hide the " + name + " window");
    addAndMakeVisible(power);
    addAndMakeVisible(fold);
    addAndMakeVisible(holder);

    editor.reset(child.createEditorIfNeeded());
    if (editor != nullptr)
    {
        holder.addAndMakeVisible(*editor);
        editor->setTopLeftPosition(0, 0);
        editor->addComponentListener(this);
    }
    applyZoom(rack.getZoom());
    addAndMakeVisible(dim);

    powerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        rack.getApvts(), MnKarpAudioProcessor::stageParamId(stage), power);
    power.onStateChange = [this] { refreshPower(); };

    fold.onClick = [this]
    {
        rack.setStageFolded(stage, ! rack.isStageFolded(stage));
        applyFolded();
        owner.updateSize();
    };

    refreshPower();
    applyFolded();
}

MnKarpAudioProcessorEditor::Section::~Section()
{
    if (editor != nullptr)
        editor->removeComponentListener(this);
    powerAttachment.reset();
}

void MnKarpAudioProcessorEditor::Section::refreshPower()
{
    dim.setVisible(! isFolded() && ! power.getToggleState());
    repaint();
}

void MnKarpAudioProcessorEditor::Section::applyFolded()
{
    fold.folded = isFolded();
    fold.repaint();
    holder.setVisible(! isFolded());
    refreshPower();
}

void MnKarpAudioProcessorEditor::Section::applyZoom(float factor)
{
    if (editor != nullptr)
        editor->setScaleFactor(factor);
}

void MnKarpAudioProcessorEditor::Section::setEditorWidth(int width)
{
    if (editor != nullptr && editor->getWidth() != width)
        editor->setSize(width, editor->getHeight());
}

juce::Point<int> MnKarpAudioProcessorEditor::Section::editorSize() const
{
    // The bounds in the parent include the editor's own scale.
    if (editor == nullptr)
        return {};

    const auto bounds = editor->getBoundsInParent();
    return { bounds.getWidth(), bounds.getHeight() };
}

int MnKarpAudioProcessorEditor::Section::preferredWidth() const
{
    return isFolded() ? 0 : editorSize().x;
}

int MnKarpAudioProcessorEditor::Section::preferredHeight() const
{
    return stripHeight + (isFolded() ? 0 : editorSize().y);
}

void MnKarpAudioProcessorEditor::Section::componentMovedOrResized(juce::Component&, bool, bool)
{
    if (! layingOut)
        owner.updateSize();
}

void MnKarpAudioProcessorEditor::Section::resized()
{
    const juce::ScopedValueSetter<bool> guard(layingOut, true);

    auto area = getLocalBounds();
    auto strip = area.removeFromTop(stripHeight);
    power.setBounds(strip.removeFromLeft(stripHeight).withSizeKeepingCentre(24, 24));
    fold.setBounds(strip.removeFromRight(stripHeight).withSizeKeepingCentre(26, 22));

    const auto size = editorSize();
    holder.setBounds(area.removeFromTop(size.y).withWidth(size.x));
    dim.setBounds(holder.getBounds());
    if (editor != nullptr)
        editor->setTopLeftPosition(0, 0);
}

void MnKarpAudioProcessorEditor::Section::paint(juce::Graphics& g)
{
    mnaudio::ui::paintCard(g, getLocalBounds());

    const bool on = power.getToggleState();
    auto strip = getLocalBounds().removeFromTop(stripHeight).withTrimmedLeft(stripHeight + 4).withTrimmedRight(stripHeight + 4);

    const juce::String name = MnKarpAudioProcessor::stageName(stage);
    const auto font = titleFont(17.0f);
    g.setFont(font);
    g.setColour(on ? Palette::accent() : Palette::dimText().withAlpha(0.6f));
    const int nameWidth = juce::GlyphArrangement::getStringWidthInt(font, name) + 6;
    g.drawText(name, strip.removeFromLeft(nameWidth), juce::Justification::centredLeft);

    g.setFont(textFont(14.0f));
    g.setColour(Palette::dimText().withAlpha(on ? 1.0f : 0.5f));
    g.drawText(role, strip.removeFromLeft(220), juce::Justification::centredLeft);

    if (! on)
    {
        g.setColour(Palette::danger());
        g.drawText("OFF - the MIDI passes through", strip, juce::Justification::centredRight);
    }
}

//==============================================================================
MnKarpAudioProcessorEditor::MnKarpAudioProcessorEditor(MnKarpAudioProcessor& processor)
    : AudioProcessorEditor(&processor), rack(processor), flowBar(processor)
{
    setLookAndFeel(&lookAndFeel);

    sections[MnKarpAudioProcessor::kordStage] = std::make_unique<Section>(
        *this, rack, MnKarpAudioProcessor::kordStage, rack.getKord(), "chords: one key plays a chord");
    sections[MnKarpAudioProcessor::arpeeStage] = std::make_unique<Section>(
        *this, rack, MnKarpAudioProcessor::arpeeStage, rack.getArpee(), "arpeggiator: pattern of the held notes");

    prevPresetButton.onClick = [this] { stepPreset(-1); };
    nextPresetButton.onClick = [this] { stepPreset(1); };
    presetButton.onClick = [this] { showPresetMenu(); };
    presetButton.setTooltip("The rack presets: the presets of mnKord and mnArpee together with the two switches");
    saveButton.setButtonText("Save");
    saveButton.onClick = [this] { saveClicked(); };
    zoomButton.setTooltip("The size of both windows");
    zoomButton.onClick = [this] { showZoomMenu(); };
    zoomButton.setButtonText(juce::String(juce::roundToInt(rack.getZoom() * 100.0f)) + "%");

    for (auto* component : std::initializer_list<juce::Component*> { &prevPresetButton, &nextPresetButton, &presetButton,
                                                                      &saveButton, &zoomButton, sections[0].get(),
                                                                      &flowBar, sections[1].get() })
        addAndMakeVisible(component);

    refreshPresetButton();
    syncWidths();
    ready = true;
    updateSize();
    startTimerHz(15);
}

MnKarpAudioProcessorEditor::~MnKarpAudioProcessorEditor()
{
    stopTimer();
    // A menu that is still open uses the look of this editor.
    juce::PopupMenu::dismissAllActiveMenus();
    // The sections own the two plugin editors; they go before the rack editor does.
    for (auto& section : sections)
        section.reset();
    setLookAndFeel(nullptr);
}

void MnKarpAudioProcessorEditor::paint(juce::Graphics& g)
{
    mnaudio::ui::paintWindow(g, getLocalBounds());

    auto brand = brandArea;
    const auto title = titleFont(24.0f);
    g.setColour(Palette::accent());
    g.setFont(title);
    const int titleWidth = juce::GlyphArrangement::getStringWidthInt(title, "mnKarp") + 4;
    g.drawText("mnKarp", brand.removeFromLeft(titleWidth), juce::Justification::centredLeft);
    g.setColour(Palette::dimText());
    g.setFont(textFont(13.0f));
    g.drawText("v" MNKARP_VERSION_STRING, brand.removeFromLeft(60), juce::Justification::centredLeft);
}

void MnKarpAudioProcessorEditor::resized()
{
    auto bar = getLocalBounds().reduced(margin, 0).removeFromTop(margin + barHeight).withTrimmedTop(margin);
    brandArea = bar.removeFromLeft(190);
    zoomButton.setBounds(bar.removeFromRight(58).withSizeKeepingCentre(58, 28));

    // The preset picker, in the middle of what is left: [<] [name] [>] [Save]
    constexpr int arrowWidth = 26, nameWidth = 240, saveWidth = 56;
    auto picker = bar.withSizeKeepingCentre(arrowWidth + 4 + nameWidth + 4 + arrowWidth + 8 + saveWidth, 30);
    prevPresetButton.setBounds(picker.removeFromLeft(arrowWidth).reduced(6, 8));
    picker.removeFromLeft(4);
    presetButton.setBounds(picker.removeFromLeft(nameWidth));
    picker.removeFromLeft(4);
    nextPresetButton.setBounds(picker.removeFromLeft(arrowWidth).reduced(6, 8));
    picker.removeFromLeft(8);
    saveButton.setBounds(picker);
    repaint();
}

//==============================================================================
int MnKarpAudioProcessorEditor::getStageEditorWidth(MnKarpAudioProcessor::Stage stage) const
{
    return sections[(size_t) stage]->editorWidth();
}

juce::Point<int> MnKarpAudioProcessorEditor::getStageVisualSize(MnKarpAudioProcessor::Stage stage) const
{
    return sections[(size_t) stage]->editorSize();
}

juce::AudioProcessorEditor* MnKarpAudioProcessorEditor::getStageEditor(MnKarpAudioProcessor::Stage stage) const
{
    return sections[(size_t) stage]->getEditor();
}

juce::String MnKarpAudioProcessorEditor::getFlowText() const
{
    return FlowBar::describe(rack.getNotesAfter(MnKarpAudioProcessor::kordStage));
}

//==============================================================================
void MnKarpAudioProcessorEditor::timerCallback()
{
    // A change of scale moves no bounds, so the sizes are looked at too.
    bool changed = false;
    for (int i = 0; i < MnKarpAudioProcessor::numStages; ++i)
    {
        const auto size = sections[(size_t) i]->editorSize();
        const bool folded = sections[(size_t) i]->isFolded();
        changed = changed || size != shownSizes[i] || folded != shownFolded[i];
    }

    if (changed)
        updateSize();

    if (getFlowText() != flowBar.shown)
        flowBar.repaint();

    if (rack.getPresetName() != shownPreset)
        refreshPresetButton();
}

void MnKarpAudioProcessorEditor::syncWidths()
{
    if (syncing)
        return;
    const juce::ScopedValueSetter<bool> guard(syncing, true);

    // A window whose width is not the rack's was resized by the user (mnArpee's corner): its width is the new one.
    int width = rack.getWidth();
    if (ready)
        for (const auto& section : sections)
            if (section->editorWidth() != rack.getWidth())
            {
                width = section->editorWidth();
                break;
            }

    rack.setWidth(width);
    for (const auto& section : sections)
        section->setEditorWidth(rack.getWidth());

    // mnArpee keeps its own limits: if it did not take the width, the rack follows it.
    for (const auto& section : sections)
        if (section->editorWidth() != rack.getWidth())
            rack.setWidth(section->editorWidth());
}

void MnKarpAudioProcessorEditor::showZoomMenu()
{
    juce::PopupMenu menu;
    menu.setLookAndFeel(&lookAndFeel);
    for (const int percent : { 60, 70, 75, 85, 100, 115 })
        menu.addItem(percent, juce::String(percent) + "%", true,
                     std::abs(rack.getZoom() * 100.0f - (float) percent) < 1.0f);

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&zoomButton),
                       [safe = juce::Component::SafePointer<MnKarpAudioProcessorEditor>(this)](int result)
                       {
                           if (safe != nullptr && result > 0)
                               safe->setZoom((float) result / 100.0f);
                       });
}

void MnKarpAudioProcessorEditor::setZoom(float factor)
{
    rack.setZoom(factor);
    zoomButton.setButtonText(juce::String(juce::roundToInt(rack.getZoom() * 100.0f)) + "%");
    for (auto& section : sections)
        section->applyZoom(rack.getZoom());
    updateSize();
}

void MnKarpAudioProcessorEditor::updateSize()
{
    if (! ready)
        return;

    syncWidths();

    int contentWidth = minContentWidth;
    for (const auto& section : sections)
        contentWidth = juce::jmax(contentWidth, section->preferredWidth());

    int y = margin + barHeight + gap;
    for (int i = 0; i < MnKarpAudioProcessor::numStages; ++i)
    {
        auto& section = *sections[(size_t) i];
        shownSizes[i] = section.editorSize();
        shownFolded[i] = section.isFolded();

        const int height = section.preferredHeight();
        section.setBounds(margin, y, contentWidth, height);
        section.resized();   // the editor inside may have changed size while the section did not
        y += height;

        if (i == MnKarpAudioProcessor::kordStage)
        {
            flowBar.setBounds(margin, y + 2, contentWidth, flowHeight);
            y += flowHeight + 4;
        }
    }

    setSize(contentWidth + 2 * margin, y + margin);
}

//==============================================================================
void MnKarpAudioProcessorEditor::refreshPresetButton()
{
    shownPreset = rack.getPresetName();
    presetButton.setButtonText(shownPreset.isEmpty() ? juce::String("(no rack preset)") : shownPreset);
}

void MnKarpAudioProcessorEditor::stepPreset(int delta)
{
    const auto names = MnKarpAudioProcessor::listPresets();
    if (names.isEmpty())
        return;

    const int current = names.indexOf(rack.getPresetName());
    const int next = current < 0 ? (delta > 0 ? 0 : names.size() - 1)
                                 : (current + delta + names.size()) % names.size();
    rack.loadPreset(names[next]);
    refreshPresetButton();
}

void MnKarpAudioProcessorEditor::showPresetMenu()
{
    rack.refreshProgramList();   // picks up files copied in from outside
    const auto names = MnKarpAudioProcessor::listPresets();
    const auto current = rack.getPresetName();

    juce::PopupMenu menu;
    menu.setLookAndFeel(&lookAndFeel);
    for (int i = 0; i < names.size(); ++i)
        menu.addItem(100 + i, names[i], true, names[i] == current);
    if (names.isEmpty())
        menu.addItem(-1, "(no rack presets yet)", false);

    menu.addSeparator();
    menu.addItem(1, "Save", current.isNotEmpty());
    menu.addItem(2, "Save as...");
    menu.addItem(3, "Delete", current.isNotEmpty() && names.contains(current));
    menu.addItem(4, "Show the preset folder");

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&presetButton),
                       [safe = juce::Component::SafePointer<MnKarpAudioProcessorEditor>(this), names, current](int result)
                       {
                           if (safe == nullptr || result <= 0)
                               return;

                           if (result >= 100)
                           {
                               safe->rack.loadPreset(names[result - 100]);
                           }
                           else if (result == 1)
                           {
                               safe->saveAs(current);
                           }
                           else if (result == 2)
                           {
                               safe->promptName(current, [safe](juce::String name)
                                                {
                                                    if (safe != nullptr)
                                                        safe->saveAs(name);
                                                });
                           }
                           else if (result == 3)
                           {
                               safe->rack.deletePreset(current);
                           }
                           else if (result == 4)
                           {
                               MnKarpAudioProcessor::getPresetFolder().createDirectory();
                               MnKarpAudioProcessor::getPresetFolder().revealToUser();
                           }
                           safe->refreshPresetButton();
                       });
}

void MnKarpAudioProcessorEditor::saveClicked()
{
    if (rack.getPresetName().isNotEmpty())
        saveAs(rack.getPresetName());
    else
        promptName({}, [safe = juce::Component::SafePointer<MnKarpAudioProcessorEditor>(this)](juce::String name)
                   {
                       if (safe != nullptr)
                           safe->saveAs(name);
                   });
}

void MnKarpAudioProcessorEditor::saveAs(const juce::String& name)
{
    if (name.isEmpty())
        return;

    // A name that is taken by another preset is only overwritten when the user says so.
    const auto names = MnKarpAudioProcessor::listPresets();
    if (name != rack.getPresetName() && names.contains(juce::File::createLegalFileName(name)))
    {
        juce::AlertWindow::showAsync(
            juce::MessageBoxOptions()
                .withIconType(juce::MessageBoxIconType::QuestionIcon)
                .withTitle("Replace the preset?")
                .withMessage("A rack preset called \"" + name + "\" exists. Replace it?")
                .withButton("Replace")
                .withButton("Cancel")
                .withAssociatedComponent(this),
            [safe = juce::Component::SafePointer<MnKarpAudioProcessorEditor>(this), name](int result)
            {
                if (safe != nullptr && result == 1)
                {
                    safe->rack.savePreset(name);
                    safe->refreshPresetButton();
                }
            });
        return;
    }

    if (! rack.savePreset(name))
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Save failed",
                                               "The preset could not be written to\n"
                                                   + MnKarpAudioProcessor::getPresetFolder().getFullPathName(),
                                               "OK", this);
    refreshPresetButton();
}

void MnKarpAudioProcessorEditor::promptName(const juce::String& initial, std::function<void(juce::String)> done)
{
    auto* window = new juce::AlertWindow("Save the rack preset", "The name of the preset:",
                                         juce::MessageBoxIconType::NoIcon, this);
    window->setLookAndFeel(&lookAndFeel);
    window->addTextEditor("name", initial);
    window->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
    window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
    window->enterModalState(true,
                            juce::ModalCallbackFunction::create(
                                [window, done = std::move(done)](int result)
                                {
                                    window->setLookAndFeel(nullptr);
                                    if (result == 1)
                                        done(window->getTextEditorContents("name").trim());
                                }),
                            true);
}
