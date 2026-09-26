#pragma once

class Info
{
public:
    Info()
    {
        /* Empty until a script sets it. Left uninitialised, getText() answered
         * whatever was in that memory. */
        text[0] = '\0';
    }

    virtual ~Info() = default;

    void setText(const char *newText)
    {
        copyString(text, newText, maxInfoTextLength);
    }

    const char *getText(void) const
    {
        return (text);
    }

private:
    static constexpr uint8_t maxInfoTextLength = 20;
    char text[maxInfoTextLength + 1];
};
