#pragma once


UIButtonDimmable touchbarToggle(
    "Touchbar", Color(0x7957FB), []() -> bool { return Device::touchbar_enable; },
    []() -> void { Device::touchbar_enable = !Device::touchbar_enable; });
AddUIComponent(touchbarToggle, Point(0, 2));