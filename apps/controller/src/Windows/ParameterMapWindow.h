#pragma once

#include "Window.h"
#include "ParameterMap.h"

class ParameterMapWindow : public Window
{
public:
    ParameterMapWindow()
    {
        parameterMap.addWindow(this);
        parameterMap.listWindows();
    }

    virtual ~ParameterMapWindow() override
    {
        parameterMap.removeWindow(this);
        parameterMap.listWindows();
    };
};
