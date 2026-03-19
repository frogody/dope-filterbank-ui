#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Reusable labeled section panel with dark background
class SectionComponent : public juce::Component
{
public:
    SectionComponent(const juce::String& title, juce::Colour accent)
        : sectionTitle(title), accentColour(accent) {}

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Panel background
        g.setColour(juce::Colour(0xff111825));
        g.fillRoundedRectangle(bounds, 4.f);

        // Subtle border
        g.setColour(juce::Colour(0x22ffffff));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 4.f, 1.f);

        // Title
        g.setColour(accentColour);
        g.setFont(12.f);
        g.drawText(sectionTitle, bounds.removeFromTop(18.f).reduced(6.f, 0.f),
                   juce::Justification::centredLeft);
    }

    juce::Rectangle<int> getContentArea() const
    {
        return getLocalBounds().reduced(4).withTrimmedTop(18);
    }

private:
    juce::String sectionTitle;
    juce::Colour accentColour;
};
