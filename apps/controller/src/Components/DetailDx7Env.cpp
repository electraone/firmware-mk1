/*
* Electra One MIDI Controller Firmware
* See COPYRIGHT file at the top of the source tree.
*
* This product includes software developed by the
* Electra One Project (http://electra.one/).
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.
*/

#include "DetailDx7Env.h"

DetailDx7Env::DetailDx7Env(const Control &control, MainDelegate &newDelegate)
    : Detail(control, newDelegate),
      l1Knob(nullptr),
      l2Knob(nullptr),
      l3Knob(nullptr),
      l4Knob(nullptr),
      r1Knob(nullptr),
      r2Knob(nullptr),
      r3Knob(nullptr),
      r4Knob(nullptr)
{
    l1Values.push_back(control.getValue(0));
    l1.setId(500);
    l1.setType(Control::Type::Knob);
    l1.setName("LEVEL 1");
    l1.setColour(control.getColour565());
    l1.setValues(l1Values);
    l1Values[0].setControl(&l1);
    l1Knob = ControlComponent::createControlComponent(l1, delegate);
    l1.setComponent(l1Knob);
    l1.addToParameterMap(l1Values[0]);
    l1Knob->updateValueFromParameterMap();
    l1Knob->assignPot(1, l1.values[0].getNumSteps());
    addAndMakeVisible(l1Knob);

    r1Values.push_back(control.getValue(1));
    r1.setId(501);
    r1.setType(Control::Type::Knob);
    r1.setName("RATE 1");
    r1.setColour(control.getColour565());
    r1.setValues(r1Values);
    r1Values[0].setControl(&r1);
    r1Knob = ControlComponent::createControlComponent(r1, delegate);
    r1.setComponent(r1Knob);
    r1.addToParameterMap(r1Values[0]);
    r1Knob->updateValueFromParameterMap();
    r1Knob->assignPot(7, r1.values[0].getNumSteps());
    addAndMakeVisible(r1Knob);

    l2Values.push_back(control.getValue(2));
    l2.setId(502);
    l2.setType(Control::Type::Knob);
    l2.setName("LEVEL 2");
    l2.setColour(control.getColour565());
    l2.setValues(l2Values);
    l2Values[0].setControl(&l2);
    l2Knob = ControlComponent::createControlComponent(l2, delegate);
    l2.setComponent(l2Knob);
    l2.addToParameterMap(l2Values[0]);
    l2Knob->updateValueFromParameterMap();
    l2Knob->assignPot(2, l2.values[0].getNumSteps());
    addAndMakeVisible(l2Knob);

    r2Values.push_back(control.getValue(3));
    r2.setId(503);
    r2.setType(Control::Type::Knob);
    r2.setName("RATE 2");
    r2.setColour(control.getColour565());
    r2.setValues(r2Values);
    r2Values[0].setControl(&r2);
    r2Knob = ControlComponent::createControlComponent(r2, delegate);
    r2.setComponent(r2Knob);
    r2.addToParameterMap(r2Values[0]);
    r2Knob->updateValueFromParameterMap();
    r2Knob->assignPot(8, r2.values[0].getNumSteps());
    addAndMakeVisible(r2Knob);

    l3Values.push_back(control.getValue(4));
    l3.setId(504);
    l3.setType(Control::Type::Knob);
    l3.setName("LEVEL 3");
    l3.setColour(control.getColour565());
    l3.setValues(l3Values);
    l3Values[0].setControl(&l3);
    l3Knob = ControlComponent::createControlComponent(l3, delegate);
    l3.setComponent(l3Knob);
    l3.addToParameterMap(l3Values[0]);
    l3Knob->updateValueFromParameterMap();
    l3Knob->assignPot(3, l3.values[0].getNumSteps());
    addAndMakeVisible(l3Knob);

    r3Values.push_back(control.getValue(5));
    r3.setId(505);
    r3.setType(Control::Type::Knob);
    r3.setName("RATE 3");
    r3.setColour(control.getColour565());
    r3.setValues(r3Values);
    r3Values[0].setControl(&r3);
    r3Knob = ControlComponent::createControlComponent(r3, delegate);
    r3.setComponent(r3Knob);
    r3.addToParameterMap(r3Values[0]);
    r3Knob->updateValueFromParameterMap();
    r3Knob->assignPot(9, r3.values[0].getNumSteps());
    addAndMakeVisible(r3Knob);

    l4Values.push_back(control.getValue(6));
    l4.setId(506);
    l4.setType(Control::Type::Knob);
    l4.setName("LEVEL 4");
    l4.setColour(control.getColour565());
    l4.setValues(l4Values);
    l4Values[0].setControl(&l4);
    l4Knob = ControlComponent::createControlComponent(l4, delegate);
    l4.addToParameterMap(l4Values[0]);
    l4Knob->updateValueFromParameterMap();
    l4Knob->assignPot(4, l4.values[0].getNumSteps());
    addAndMakeVisible(l4Knob);

    r4Values.push_back(control.getValue(7));
    r4.setId(507);
    r4.setType(Control::Type::Knob);
    r4.setName("RATE 4");
    r4.setColour(control.getColour565());
    r4.setValues(r4Values);
    r4Values[0].setControl(&r4);
    r4Knob = ControlComponent::createControlComponent(r4, delegate);
    r4.setComponent(r4Knob);
    r4.addToParameterMap(r4Values[0]);
    r4Knob->updateValueFromParameterMap();
    r4Knob->assignPot(10, r4.values[0].getNumSteps());
    addAndMakeVisible(r4Knob);

    Detail::setLocked();
}

DetailDx7Env::~DetailDx7Env()
{
    l1.removeFromParameterMap(l1Values[0]);
    r1.removeFromParameterMap(r1Values[0]);
    l2.removeFromParameterMap(l2Values[0]);
    r2.removeFromParameterMap(r2Values[0]);
    l3.removeFromParameterMap(l3Values[0]);
    r3.removeFromParameterMap(r3Values[0]);
    l4.removeFromParameterMap(l4Values[0]);
    r4.removeFromParameterMap(r4Values[0]);
    l1Knob->releasePot();
    l2Knob->releasePot();
    l3Knob->releasePot();
    l4Knob->releasePot();
    r1Knob->releasePot();
    r2Knob->releasePot();
    r3Knob->releasePot();
    r4Knob->releasePot();
}

void DetailDx7Env::paint(Graphics &g)
{
    g.setColour(Colours565::black);
    g.fillRect(0, 0, getWidth() - 120, getHeight());
    g.setColour(Colours565::darkgrey);
    g.drawRect(0, 0, getWidth() - 120, getHeight());
    g.dim(getWidth() - 120, 0, 120, getHeight(), Colours565::black);
}

void DetailDx7Env::resized()
{
    component->setBounds(50, 50, 608, 150);
    lock->setBounds(getWidth() - 100, 25, 75, 75);
    dimmedSection->setBounds(getWidth() - 120, 100, 120, getHeight() - 100);
    l1Knob->setBounds(63, getHeight() - 240, 75, 100);
    l2Knob->setBounds(228, getHeight() - 245, 75, 100);
    l3Knob->setBounds(394, getHeight() - 245, 75, 100);
    l4Knob->setBounds(560, getHeight() - 245, 75, 100);
    r1Knob->setBounds(63, getHeight() - 130, 75, 100);
    r2Knob->setBounds(228, getHeight() - 130, 75, 100);
    r3Knob->setBounds(394, getHeight() - 130, 75, 100);
    r4Knob->setBounds(560, getHeight() - 130, 75, 100);
}