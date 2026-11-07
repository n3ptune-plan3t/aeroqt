# AeroQt — handoff (Phase 12 done: built-in themes now really load, HiDPI cache fixed, QML style loads + is tested; GTK excluded)

Goal: from-scratch Qt6/KDE widget style ("aeroqt"), Frutiger Aero default, modular,
easy to debug/style, compatible with KDE (palette-driven, no KDE link deps). Qt5 dropped.

## Architecture
- AeroStyle : QProxyStyle("fusion")  -> unhandled elements fall back to Fusion (always usable)
- Module (src/module.h): one per widget family. Hooks (all optional):
    draw:      drawPrimitive / drawControl / drawComplex      -> return true = handled
    geometry:  pixelMetric (-1) / sizeFromContents (invalid QSize) / subControlRect (null QRect)
               / subElementRect (null QRect) / styleHint (Module::Unhandled)
  `Module::self` = the AeroStyle. Modules call `self->subControlRect(...)` to get FINAL geometry
  (own override or Fusion's). Dispatch order = registration order in AeroStyle ctor.
- Theme (JSON tokens, "@role" palette colors, "@a+@b:0.3" mixes) -> KDE color schemes work
- Glass (src/glass.cpp): the ONLY place the Aero look is painted:
    drawSlab (glass body, Spec.axis = Vertical|Horizontal gradient direction), drawInset,
    drawFocusGlow, drawArrow, drawGrip
- Theme override: $AEROQT_THEME or ~/.config/aeroqt/theme.json (deep-merged, hot-reloaded)
- Debug: AEROQT_DEBUG=1 logs which module handled / fell back per element
- Visual test: QT_QPA_PLATFORM=offscreen ./build/aeroqt-preview out.png [--dark] [--page=1|2]
  (page 1 = buttons/inputs/progress, page 2 = scrollbar/slider/spin/combo, page 3 = tabs+menus). Then LOOK at the PNG.

## Done (compiles on Qt 6.4.2, verified visually light+dark)
Phase 2b: tabs (all 4 orientations, selected tab merges into pane, standalone base line with gap),
menu popup (panel/frame/items/separators/icons/check/radio/shortcut/submenu arrow, RTL via visualRect),
menubar (glass bar + glass active item). New Glass helpers: drawPanel, drawBar, drawSeparator.
buttons, line edit, checkbox, radio, progress groove+fill,
scrollbar (inset track, glass thumb + grip, glass arrow buttons),
slider (thin groove, accent fill, round handle, ticks; horiz/vert/inverted),
spinbox (unified glass button column + divider), combobox (non-editable = glass button;
editable = inset + button column).

## Not yet verified (no interactive session available headless)
hover/pressed visuals for the new widgets (and tab hover) are code-reviewed only. Test in a real
session (`QT_STYLE_OVERRIDE=aeroqt`) or add a QTest mouse-event script.

Phase 3: frames.cpp (CC_GroupBox, PE_Frame, PE_PanelTipLabel, PE_FrameFocusRect), itemviews.cpp (item/row
selection+hover slabs, CE_HeaderSection), docks.cpp (CE_Splitter, dock resize handle, dock title, floating frame).
New: Theme::contrastText(), Glass::drawHandleDots(). polish(): WA_Hover on view viewports + QSplitterHandle.
Preview: --page=4 (frames+views); item states / splitter handles / tooltip are painted directly in preview.cpp.

Phase 4: bars.cpp (CE_ToolBar, toolbar handle/separator, status bar panel, CE_SizeGrip), misc.cpp (PE_IndicatorBranch,
CC_Dial, CE_ToolBoxTabShape, CE_RubberBand), menus.cpp (+ section headers, CE_MenuScroller). New: Glass::edgeLight().
Preview: --page=5 (bars+misc); size grips, rubber band, menu scroller and toolbar handle/separator are painted directly.

Phase 5: animation engine in AeroStyle (implements `Animator`, declared in module.h; modules get it as `anim`,
and the helper `Module::fade(w, id, on)`). Glass::Spec gained `hoverT` (0..1 from the engine) + `hoverTint`
(fraction of `glow` mixed into `base`) — modules no longer do Theme::mix(base, accent, .15) for hover themselves.
Faded: buttons/tool buttons (flat ones fade in/out entirely), check/radio, scrollbar parts, slider handle, spin/combo,
tabs (per tab), menu + menubar items (highlight glides), item-view hover slabs, header sections, toolbox tabs, dial.
Progress: indeterminate marquee (min==max==0) + a soft shine sweep on running determinate bars.
Tokens: anim.enabled/hoverIn/hoverOut, progress.marqueeMs/shine/shineMs. Kill switch: env AEROQT_NO_ANIM=1.
Test: `aeroqt-animtest` (ctest: anim_on / anim_off) checks fade monotonicity, fade-out to rest, marquee motion, kill switch.

Phase 6: titlebar.cpp (CC_TitleBar glass bar + delegated button slabs + glyphs), bars.cpp
PM_ToolBarExtensionExtent + standardPixmap double-chevron extension icon, menus.cpp CE_MenuTearoff engraved line+dots;
frames.cpp PE_Frame gained focus glow
(State_HasFocus -> Glass::drawFocusGlow) covering QTextEdit/QAbstractScrollArea/calendar/wizard pages.
QCalendarWidget/QWizard/QProgressDialog need no painter: frame + tool/push buttons + progress bar already Aero;
polish() sets WA_Hover on QCalendarWidget. Tokens: titlebar.base/active/border/radius/height.
Preview: --page=6 (MDI subwindow, calendar, focused text edit, painted title bar/extension/tear-off,
wizard + progress dialog grabbed to pixmaps). Qt5 removed: CMake Qt6-only, menus.cpp icon guard dropped.

Phase 7: themes/flat-dark.json + themes/win7-classic.json (complete standalone token sets, in themes.qrc).
Selection: $AEROQT_THEME_NAME=flat-dark|win7-classic (Theme::builtinPath(); default frutiger-aero);
user $AEROQT_THEME / ~/.config/aeroqt/theme.json deep-merges over the SELECTED built-in (not always frutiger).
Preview: --theme=flat-dark|win7-classic (must be parsed before AeroStyle ctor). KDE compat: no KDE link deps,
all colors are "@role" palette mixes, so KDE color schemes (kdeglobals) flow through the palette automatically;
only progress fill/glow are fixed hex accents. Test in a real KDE session (Dolphin/Kate) - not headless.

## Theme tokens (themes/*.json, dotted keys; colors are "#rrggbb"/"#aarrggbb"/"@role"/"@a+@b:t", numbers are qreal)
name, gloss, accent | button.base/border/glow/radius | input.base/border/focus/radius | check.base/mark/radius
progress.groove/fill/glow/radius/marqueeMs/shine/shineMs | anim.enabled/hoverIn/hoverOut
scrollbar.groove/thumb/border/grip/radius/extent/minThumb | slider.groove/fill/handle/border/tick/grooveWidth/handleLength/handleThickness
spin.button/radius/buttonWidth | combo.arrow/buttonWidth | tab.base/selected/pane/border/radius/paneRadius
menu.base/border/separator/item/itemBorder/itemRadius/itemHeight/separatorHeight/hMargin/vMargin
menubar.base/border | group.base/border/radius | view.selected/inactive/hover/radius | header.base/border
splitter.width/dots | dock.base/border | toolbar.base/border/handle/separator/extension | statusbar.base/border
titlebar.base/active/border/radius/height

Phase 8: qml/QtQuick/Controls/AeroQt/ QQC2 style (Button/CheckBox/RadioButton/ComboBox/TextField/TextArea/
Slider/ProgressBar/BusyIndicator/ScrollBar/TabBar/ToolBar/SpinBox/Switch, +qmldir) for Kirigami/QML KDE apps.
Colors come from control.palette (QPalette roles), so KDE color schemes flow through exactly like the Widgets side;
look = rounded (r=6) palette.button base + white gloss top-half + highlight bottom glow + dark border.
Install: CMake installs it to <libdir>/qt6/qml/AeroQt (a top-level module, see Phase 12). Enable BOTH layers with
tools/aeroqt-env.sh (installed to share/aeroqt/): QT_STYLE_OVERRIDE=aeroqt (Widgets) +
QT_QUICK_CONTROLS_STYLE=AeroQt (QML). Source it / add to ~/.profile, re-login. GTK apps unaffected (GTK themes).

Phase 9: theming polish against upstream docs (no local compile; verified via GitHub Actions).
Docs consulted: QPalette color roles (https://doc.qt.io/qt-6/qpalette.html) — use symbolic roles
rather than literal colors like "red"; QProxyStyle/custom-style guide
(https://doc.qt.io/qt-6/qstyle.html#creating-a-custom-style) — wrap the platform style, delegate
unhandled elements to Fusion; High DPI (https://doc.qt.io/qt-6/highdpi.html) — Widgets/Quick use a
device-independent coordinate system but hairlines/pixmaps must respect the physical pixel grid via
QPaintDevice::devicePixelRatioF / QPixmap::setDevicePixelRatio; QQC2 customization
(https://doc.qt.io/qt-6/qtquickcontrols2-customize.html) — every state (pressed/hovered/visualFocus/
enabled/checked) must be visibly distinct, use visualFocus (not activeFocus) for the focus border;
Kirigami colors (https://develop.kde.org/docs/getting-started/kirigami/style-colors) + KDE HIG
(https://hig.kde.org/) — hard-coded colors break light/dark schemes, pull from the active scheme
and keep contrast/accessibility (disabled/focus never color-alone).
Changes: (1) theme.cpp now resolves the full QPalette::ColorRole set (alternateBase, toolTipBase/
Text, link/linkVisited, midlight, placeholderText) and clamps mix weights, so "@alternateBase"
etc. no longer silently falls back; (2) progress fill/glow are palette-derived "@highlight" /
"@highlight+@light:0.45" in all 3 themes (was fixed # hex, which stayed blue/green on every KDE
scheme) + QML ProgressBar ditto; (3) Glass snap (0.5/dpr) + edgeLight-scaled keyline + luminance-
grey disabled tint; toolbar-extension pixmap is now DPR-backed and uses the Disabled palette group;
(4) QML: visualFocus borders (Button/CheckBox/Radio/Combo/Slider/SpinBox/Switch/ScrollBar/
TextField/TextArea), checked/hovered/pressed/disabled/placeholder states, ComboBox arrow rotates
when open, Switch knob animates, ProgressBar fill clipped to the groove.

Phase 10: lightweight Aero (same look, fewer cycles) — comparison vs standards.
Docs/source basis: Qt Styling Approaches (https://doc.qt.io/qt-6/qtwidgets-styling-approaches.html):
a style subclass repaints natively, QSS instead "matches selectors ... caches rules per widget, the
memory/cost grows with widgets+rules" and repolishes everything; Fusion "draws with QPalette, same
style light+dark variant, no switch"; Kvantum = SVG-based engine (needs QtSvg, parses+renders SVG
themes at runtime), Klassy/Breeze = binary C++ QStyle forked from Breeze but with KF6 deps
(KF6KirigamiPlatform/KF6WindowSystem/Qt6Svg...), Lightly likewise. AeroQt stays: QProxyStyle over
built-in Fusion (no reimplemented geometry), QPainter only (no QtSvg/KF/QSS — no extra link deps,
.handoff Phase 7), full QPalette roles. What Phase 10 trims to approach Fusion/Klassy cost:
- Theme hot path: Theme::lookup() + color-string split are cached after load (was split+trim+toDouble
  per paint). Still palette-resolved per call, but string work is gone — the overhead Fusion avoids.
- Glass: perf.gloss/perf.glow multipliers (1 = full); 0 skips that gradient pass entirely. Settled
  slabs (hoverT at rest) are rendered once to a DPR-aware QPixmap (cap 96, ~1-2MB) and blitted after,
  incl. caller clip (tabs); mid-transition hover frames (110/240ms) still paint direct. Breeze/Klassy
  cache the static state the same way; Kvantum re-renders SVG instead.
- Animator: QTimer is now CoarseTimer + anim.frameMs (default 16, aero-lite 33 ≈30fps). Same
  hoverIn/Out durations, half the wakeups. AEROQT_PERF_LIGHT forces frameMs≥33 + gloss/glow≤0.3
  on any theme (Klassy-style "reduce effects" preset). AEROQT_NO_ANIM still kills hover animation.
- New built-in preset aero-lite (AEROQT_THEME_NAME=aero-lite, --theme=aero-lite): Win7-flavoured
  geometry, gloss 0.12, perf 0.3/0.3, progress.shine=0 (no 60fps shine repaint), faster short hover
  (80/160ms). QML mirrors this implicitly (palette-driven, no
  extra QML backend; Kirigami animations stay at the app's own rate).

Phase 11: the other 3 themes are lightweight too, signature untouched. gloss/radius/border/
colors in frutiger-aero, flat-dark and win7-classic are unchanged (Frutiger keeps gloss 0.55 +
shine 1 as its signature; Flat keeps gloss 0.3 and its flat look; Win7 keeps gloss 0.4/classic
radius 3). The saving is code-side and applies to every theme: Theme lookup/split cache + settled-
slab pixmap cache + CoarseTimer mean the expensive path runs once and blits after. Themes only
add: anim.frameMs=25 (≈40fps, same 110/240ms hover durations) + perf — Frutiger 1.0/1.0 (full
sign, paid once via cache), Flat 0.7/0.7 + progress.shine=0 (flat = no sweep), Win7 0.85/0.85 +
shine=0 (classic = no sweep). aero-lite stays the lowest (0.3/0.3 + 33ms).

## Gotchas learned in 8
- QQC2 style != QStyle: a Widgets-only plugin can never restyle Kirigami/QML controls. The QML side needs its
  own style directory + qmldir, selected by QT_QUICK_CONTROLS_STYLE (not QT_STYLE_OVERRIDE).
- QML controls read control.palette.* (QPalette roles), not our JSON tokens - so QML follows KDE scheme
  automatically, but JSON gloss/radius choices are mirrored by hand in QML (r=6, gloss 0.28). Keep in sync.
- (Phase 12: the QML style is now loaded + behaviour-tested headless by ctest "qml"; still try a real Kirigami app.)

## Gotchas learned in 7
- Built-in themes are COMPLETE files: load() loads :/aeroqt/*.json as-is, only real file paths deep-merge.
  A user override therefore always lands on the selected built-in (builtinPath()), never just frutiger.
- Preview must parse --theme= BEFORE `new AeroStyle` (ctor reads AEROQT_THEME_NAME via builtinPath()).

## Gotchas learned in 6
- CC_TitleBar: paint the bar, then delegate each button slab via self->drawPrimitive(PE_PanelButtonTool)
  (buttons module dispatches again - no recursion for a different element) and draw only the glyph.
  Geometry comes from self->subControlRect(CC_TitleBar, ...) = Fusion's - no subControlRect override needed.
- PE_Frame is the QFrame border hook: adding the focus glow there lights up QTextEdit, scroll areas,
  calendar and wizard pages at once. Line edits already glow via PE_PanelLineEdit (inputs.cpp).
- Wizard/progress dialog in preview: top-level with no parent, show() then grab() then hide() - same
  offscreen pattern as QMenu popup (a parent without a window handle crashes).
- Toolbar extension is NOT a style primitive (no PE_IndicatorToolBarExtension in QStyle): QToolBar shows a
  QToolButton with SP_ToolBarHorizontal/VerticalExtensionButton icon sized by PM_ToolBarExtensionExtent.
  Aero styles it via AeroStyle::standardPixmap (double chevron) + buttons.cpp slab. CE_MenuTearoff has a
  lowercase f - CE_MenuTearOff does not exist.
- State_Horizontal on PE_IndicatorToolBarHandle means the BAR is horizontal (arrow points right).

## Gotchas learned in 5
- hover(w, id, on): new (widget,id) entries start at 0 and fade toward the target; entries at rest AND 0 are erased in tick().
  Never call update() from inside a paint: only the 16ms timer repaints (tick), and it stops itself when nothing moves.
- Scroll areas: the widget handed to the style is the VIEW, but painting happens in its viewport -> repaintWidget() updates
  the viewport. Do the same if you add another animated scroll-area widget.
- phase(w, ms) registers w as "animating" for 250ms after the last call; just stop calling it to stop the timer.
  Every running determinate progress bar repaints at ~60fps because of the shine: disable with progress.shine = 0.
- Hover ids: tabs/headers/toolbox use rect position, menus use item top/left, item views use row/col/internalId.
- Selection in item views is instant on purpose (only hover fades).
- Opacity trick for fade-out of slabs: p->setOpacity(p->opacity() * h) around Glass::drawSlab.

## Gotchas learned in 4
- Engraved highlights (white 1px line) must be SOFT on dark themes: use Glass::edgeLight(bg) (0.7 light / 0.1 dark).
- Menu section header (separator with text): QMenu passes 2x2 to CT_MenuItem, the style must add the text width itself.
- Toolbar border sits on the side facing the content (toolBarArea); status bar = same bar mirrored (top border).
- Dial angle formula = QStyleHelper::drawDial: wrapping 3pi/2 - p*2pi, else (8pi - p*10pi)/6. Handle ring = 0.68 knob radius.
- Branch indicator: no connector lines on purpose; State_MouseOver only arrives because polish() sets WA_Hover on view viewports.
- SH_RubberBand_Mask = 0: the band blends with alpha (a top-level rubber band without compositing would show a solid box).
- A stray empty dir "{src" (from a brace-expansion tar mishap) was removed from the tree.

## Gotchas learned in 3
- Qt item views report State_Active ONLY while the view has keyboard focus (even in an active window).
  So "unfocused view" == "inactive selection": we draw a calm grey slab (token view.inactive), no accent glow.
- CE_ItemViewItem: stock HighlightedText (white) vanishes on our LIGHT selection glass. itemviews.cpp re-enters the
  style once with a copied option whose HighlightedText = Theme::contrastText(base); a thread_local `busy` flag makes the
  module step aside on the re-entry so Fusion draws the content. Reuse this pattern for any "tweak option then delegate".
- QTableView never sets viewItemPosition; positionOf() derives it from the column so a selected row is ONE slab.
- PE_PanelItemViewRow only paints backgrounds (alternate/brush); items draw the slabs (avoids a glow blob in tree branch area).
- Tooltip is square on purpose (tip window has no translucency). PE_IndicatorHeaderArrow left to Fusion
  (Qt's SortUp/SortDown naming is inverted vs. intuition). Vertical dock title bars fall back to Fusion.
- Offscreen preview: a view only paints 'active' selection if it has focus -> preview focuses the list after show.
- GroupBox label uses subControlRect from Fusion; flat group box = engraved line at frame top.

## Gotchas learned in 2b
- QMenu adds tabWidth (widest shortcut) to the column width itself: CT_MenuItem must NOT include it.
- QMenuBar paints items + empty area only (no panel): both CE_MenuBarItem and CE_MenuBarEmptyArea paint the bar.
- Glass::drawSlab/drawInset now INTERSECT clips (no setClipping(false)): callers may clip a slab (tabs do).
- Preview: QMenuBar::setActiveAction() pops the submenu up -> segfault offscreen; QMenu needs no parent + popup() before grab().
- Tab bar overlap: PM_TabBarBaseOverlap=1 so the pane border runs under the bar's last row; selected tab covers it.
- Section headers (separator with text), menu scroller, tear-off still fall back to Fusion.

Phase 12: audit with a real Qt 6.4 build (earlier phases were only verified through CI). Findings + fixes:
- BUILT-IN THEMES NEVER LOADED. themes.qrc is inside the static lib aeroqt_core; nothing referenced its
  initializer, so the linker dropped it and ":/aeroqt/*.json" did not exist in the plugin or any tool. Every
  module ran on its hard-coded fallbacks: frutiger/flat-dark/win7-classic/aero-lite rendered byte-identical,
  and perf.*, anim.frameMs, progress.shine had no effect. Fix: Q_INIT_RESOURCE(themes) in theme.cpp.
  The look of the default theme is now what the JSON says (it was the module fallbacks before).
- Glass slab cache was half size at DPR>1 (see the Phase 10 gotcha). Now: physical-size pixmap, POD key
  (no per-paint QString building), QCache LRU with a ~6 MB budget, cleared in ~AeroStyle.
- Animator: hover() inserted state + a destroyed() connection for every painted part even when never hovered
  (item views: every cell); entries were erased at rest so the same widget re-connected on each later hover
  (connections piled up). Now cold parts allocate nothing and connections are Qt::UniqueConnection;
  unpolish() forgets the widget. AeroStyle::trackedParts() exists for the tests.
- Theme: lookup("name.x") returned the string when a path walked through a non-object; user overrides
  merged one level deep only (now recursive); unknown AEROQT_THEME_NAME is case-insensitive + warns once;
  bad user theme keeps the previous one; deleting theme.json reverts to the built-in; the watcher also
  watches the directory (a theme.json created after login hot-reloads) with a 60 ms debounce.
- Flat tool buttons showed no keyboard focus indicator when idle; now they do.
- QML style: could not load at all. (1) A custom style is a top-level module named after the style:
  qml/AeroQt/qmldir ("module AeroQt"), not QtQuick/Controls/AeroQt (Qt: 'module "AeroQt" is not installed').
  (2) ComboBox: missing T.Popup; showed blank text (editText instead of displayText); no delegate and
  no popup height (empty dropdown); (3) SpinBox: upIndicator/downIndicator do not exist (up.indicator /
  down.indicator) and an editable box needs a TextInput contentItem; (4) T.TextField/T.TextArea draw no
  placeholder themselves - the style must supply a PlaceholderText item; (5) added TabButton (TabBar had none).
- Tests: aeroqt-coretest (ctest "core": color/token parsing, deep merge, HiDPI cache at DPR 1/1.5/2/3, animator
  bookkeeping) and aeroqt-qmltest (ctest "qml": loads every control with 0 QML warnings + behaviour checks;
  built only if Qt6 Quick/QuickControls2 are found). Both were mutation-checked against the old code.
  QML test deps (Ubuntu): qt6-declarative-dev qml6-module-qtquick{,-controls,-templates,-layouts,-window}
  qml6-module-qtqml-workerscript. Run with QT_QUICK_BACKEND=software under QT_QPA_PLATFORM=offscreen.
- Gotcha: static-lib Qt resources need Q_INIT_RESOURCE from a referenced object, or they silently vanish.
- Gotcha: when checking "does X render differently", set AEROQT_NO_ANIM=1 (the progress shine/marquee make
  repeated renders differ) and compare md5s of the PNGs.
- Not covered by anything: other QML controls (Menu, ItemDelegate, Dial, GroupBox...) still fall back to Qt's Basic.

## TODO (priority order)
2. (done) animations. Possible follow-ups: press ripple, tab switch crossfade, QStackedWidget fade, tooltip fade-in
3. (done) remaining widgets (Phase 6); QLCDNumber needs nothing
4. (done, code) KDE compat: palette-only colors, no KDE link deps (Phase 7); QML apps via QQC2 style (Phase 8).
   Real-session test still TODO: Dolphin/Kate (QT_STYLE_OVERRIDE=aeroqt) + a Kirigami app
   (QT_QUICK_CONTROLS_STYLE=AeroQt). GTK excluded on purpose.
5. (done) extra themes flat-dark + win7-classic + token docs (Phase 7)
6. Wayland/HiDPI check (all painting is vector, uses QRectF); verify with devicePixelRatio test
   (Phase 9: Glass snap + extension pixmap are DPR-aware; still no live Wayland test headless)

## Gotchas learned in 10
- Slab cache key must include hoverT + hoverTint: settled hv=0 vs hv=1 share Spec.base but differ
  by hoverTint mixing inside drawSlabDirect; without them the two states collide in one pixmap.
- QPixmap(w, h) is w x h *physical* pixels; setDevicePixelRatio(dpr) then makes its logical size
  (w/dpr, h/dpr). The slab cache must therefore allocate logical*dpr pixels (Phase 12 fix: it used to
  allocate logical size, so slabs were half size at DPR 2). Paint in logical coords, no extra p.scale(dpr).
- CoarseTimer tick: hover math uses elapsed dt, so frameMs 16→33 does not change total duration,
  only the step size. Keep tick interval === theme anim.frameMs and update it in applyAnimSettings.

## Gotchas learned in 9
- Hard-coded hex in theme tokens (progress fill/glow) breaks "follows the KDE scheme": palette
  role mixes ("@highlight+@light:0.45") derive the same hue in light and dark, per QPalette docs
  ("use color group roles rather than literal colors") and Kirigami "avoid hard-coded colors".
- QML: activeFocus != visualFocus. Styling the focus border from activeFocus shows a ring on
  mouse clicks; Qt docs customize samples gate the border on visualFocus. Disabled = content +
  background dim, not just border grey (color-alone fails accessibility).
- devicePixelRatioF: QPaintDevice::devicePixelRatioF (widgets) / QScreen::devicePixelRatio (pixmap
  fallback). QPixmap painted at 16x16 logical with setDevicePixelRatio(dpr) is automatically
  drawn at dpr physical pixels — do NOT p.scale(dpr) on top or it double-scales.

## Conventions
- New widget = src/modules/<name>.cpp + register<Name>() declared in module.h + AeroStyle ctor + CMake list
- Never hardcode colors in modules: add a token to themes/frutiger-aero.json, read via Theme::color(key, pal, fallback)
- Soft arrows: mix ButtonText with Button 25% (see spincombo.cpp arrowColor)
- Install: plugin -> <libdir>/qt6/plugins/styles/libaeroqt.so; use with `QT_STYLE_OVERRIDE=aeroqt` or `-style aeroqt`

## Dev environment recipe (Debian/Ubuntu, worked in sandbox)
  apt-get install -y cmake ninja-build qt6-base-dev libgl1-mesa-dev
  # optional, for the QML test (ctest "qml"):
  apt-get install -y qt6-declarative-dev qml6-module-qtquick qml6-module-qtquick-controls \
      qml6-module-qtquick-templates qml6-module-qtquick-layouts qml6-module-qtquick-window qml6-module-qtqml-workerscript
  cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build
  QT_QPA_PLATFORM=offscreen ./build/aeroqt-preview /tmp/x.png --page=2
