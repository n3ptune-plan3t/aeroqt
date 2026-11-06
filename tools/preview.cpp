// aeroqt-preview [out.png] [--dark] [--theme=flat-dark|win7-classic] [--page=N]  (1 basics, 2 range widgets, 3 tabs+menus, 4 frames+views, 5 bars+misc, 6 remaining)  -- renders a widget gallery with AeroStyle.
// Works headless: QT_QPA_PLATFORM=offscreen aeroqt-preview out.png
// Pages group widgets so each PNG stays readable; default = all pages stitched side by side.
#include <QApplication>
#include <QCheckBox>
#include <QActionGroup>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QStyleOptionMenuItem>
#include <QTabBar>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QTextEdit>
#include <QSplitter>
#include <QMainWindow>
#include <QDockWidget>
#include <QHeaderView>
#include <QStyleOptionViewItem>
#include <QStyleOptionFrame>
#include <QDial>
#include <QToolBox>
#include <QCalendarWidget>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QProgressDialog>
#include <QToolBar>
#include <QWizard>
#include <QWizardPage>
#include <QStatusBar>
#include <QSizeGrip>
#include <QStyleOptionSizeGrip>
#include <QStyleOptionRubberBand>
#include <QStyleOptionToolBar>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPalette>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QSlider>
#include <QSpinBox>
#include <QToolButton>
#include "aerostyle.h"

static void setPalette(QApplication &app, bool dark)
{
    QPalette p;
    if (dark) {
        p.setColor(QPalette::Window, QColor(40, 44, 52));
        p.setColor(QPalette::WindowText, Qt::white);
        p.setColor(QPalette::Base, QColor(28, 31, 38));
        p.setColor(QPalette::Text, Qt::white);
        p.setColor(QPalette::Button, QColor(58, 64, 76));
        p.setColor(QPalette::ButtonText, Qt::white);
        p.setColor(QPalette::Dark, QColor(15, 17, 21));
        p.setColor(QPalette::Mid, QColor(52, 58, 70));
        p.setColor(QPalette::AlternateBase, QColor(34, 38, 46));
        p.setColor(QPalette::ToolTipBase, QColor(52, 58, 70));
        p.setColor(QPalette::ToolTipText, Qt::white);
        p.setColor(QPalette::Highlight, QColor(42, 130, 218));
        p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(120, 125, 135));
        p.setColor(QPalette::Disabled, QPalette::Text, QColor(120, 125, 135));
        p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(120, 125, 135));
    } else {
        p.setColor(QPalette::Window, QColor(214, 232, 246));
        p.setColor(QPalette::Button, QColor(226, 238, 248));
        p.setColor(QPalette::Highlight, QColor(40, 150, 230));
        p.setColor(QPalette::Dark, QColor(90, 110, 130));
        p.setColor(QPalette::AlternateBase, QColor(238, 245, 251));
        p.setColor(QPalette::ToolTipBase, QColor(255, 255, 225));
        p.setColor(QPalette::ToolTipText, Qt::black);
    }
    app.setPalette(p);
}

static QWidget *basicsPage()
{
    auto *w = new QWidget;
    auto *f = new QFormLayout(w);
    f->addRow("Buttons", new QPushButton("Push button"));
    auto *def = new QPushButton("Default"); def->setDefault(true);
    f->addRow("Default", def);
    auto *dis = new QPushButton("Disabled"); dis->setEnabled(false);
    f->addRow("Disabled", dis);
    auto *tb = new QToolButton; tb->setText("Tool"); f->addRow("Tool", tb);
    f->addRow("Line edit", new QLineEdit("Hello Aero"));
    auto *c1 = new QCheckBox("Checked"); c1->setChecked(true);
    auto *c2 = new QCheckBox("Unchecked");
    auto *r1 = new QRadioButton("Radio on"); r1->setChecked(true);
    f->addRow(c1); f->addRow(c2); f->addRow(r1); f->addRow(new QRadioButton("Radio off"));
    auto *pb = new QProgressBar; pb->setValue(65); f->addRow("Progress", pb);
    auto *pbBusy = new QProgressBar; pbBusy->setRange(0, 0); f->addRow("Busy (marquee)", pbBusy);
    auto *pbV = new QProgressBar; pbV->setOrientation(Qt::Vertical); pbV->setValue(40); pbV->setMaximumHeight(50); f->addRow("Vertical", pbV);
    return w;
}

static QWidget *rangePage()
{
    auto *w = new QWidget;
    auto *h = new QHBoxLayout(w);
    auto *f = new QFormLayout;
    h->addLayout(f);

    auto *hs = new QScrollBar(Qt::Horizontal); hs->setRange(0, 100); hs->setPageStep(30); hs->setValue(25);
    f->addRow("ScrollBar H", hs);
    auto *hsd = new QScrollBar(Qt::Horizontal); hsd->setRange(0, 100); hsd->setValue(25); hsd->setEnabled(false);
    f->addRow("  disabled", hsd);

    auto *s1 = new QSlider(Qt::Horizontal); s1->setValue(40); f->addRow("Slider", s1);
    auto *s2 = new QSlider(Qt::Horizontal); s2->setValue(70);
    s2->setTickPosition(QSlider::TicksBelow); s2->setTickInterval(10); f->addRow("  ticks below", s2);
    auto *s3 = new QSlider(Qt::Horizontal); s3->setValue(30);
    s3->setTickPosition(QSlider::TicksBothSides); s3->setTickInterval(25); f->addRow("  ticks both", s3);
    auto *s4 = new QSlider(Qt::Horizontal); s4->setValue(50); s4->setEnabled(false); f->addRow("  disabled", s4);
    auto *s5 = new QSlider(Qt::Horizontal); s5->setValue(30); s5->setInvertedAppearance(true); f->addRow("  inverted", s5);

    auto *sp = new QSpinBox; sp->setValue(42); f->addRow("SpinBox", sp);
    auto *dsp = new QDoubleSpinBox; dsp->setValue(3.14); dsp->setSuffix(" cm"); f->addRow("DoubleSpin", dsp);
    auto *spd = new QSpinBox; spd->setValue(7); spd->setEnabled(false); f->addRow("  disabled", spd);
    auto *spm = new QSpinBox; spm->setRange(0, 10); spm->setValue(10); f->addRow("  at max", spm);

    auto *cb = new QComboBox; cb->addItems({"Frutiger Aero", "Flat Dark", "Win7 Classic"}); f->addRow("ComboBox", cb);
    auto *ce = new QComboBox; ce->setEditable(true); ce->addItems({"Editable", "Other"}); f->addRow("  editable", ce);
    auto *cd = new QComboBox; cd->addItems({"Disabled"}); cd->setEnabled(false); f->addRow("  disabled", cd);

    auto *vs = new QScrollBar(Qt::Vertical); vs->setRange(0, 100); vs->setPageStep(35); vs->setValue(20);
    vs->setMinimumHeight(260);
    h->addWidget(vs);
    auto *vsl = new QSlider(Qt::Vertical); vsl->setValue(60);
    vsl->setTickPosition(QSlider::TicksRight); vsl->setTickInterval(20);
    h->addWidget(vsl);
    auto *vsl2 = new QSlider(Qt::Vertical); vsl2->setValue(30); h->addWidget(vsl2);
    return w;
}

static QWidget *miscPage()
{
    auto *w = new QWidget;
    auto *v = new QVBoxLayout(w);

    // main window with toolbar (top + left), status bar
    auto *mw = new QMainWindow;
    auto *st = w->style();
    auto *tb = mw->addToolBar("Main");
    tb->addAction(st->standardIcon(QStyle::SP_DialogOpenButton), "Open");
    tb->addAction(st->standardIcon(QStyle::SP_DialogSaveButton), "Save");
    tb->addSeparator();
    auto *chk = tb->addAction(st->standardIcon(QStyle::SP_FileIcon), "Toggle"); chk->setCheckable(true); chk->setChecked(true);
    tb->addAction(st->standardIcon(QStyle::SP_TrashIcon), "Delete");
    auto *lt = new QToolBar("Side");
    lt->addAction(st->standardIcon(QStyle::SP_ArrowUp), "Up");
    lt->addSeparator();
    lt->addAction(st->standardIcon(QStyle::SP_ArrowDown), "Down");
    mw->addToolBar(Qt::LeftToolBarArea, lt);
    mw->setCentralWidget(new QTextEdit("central widget"));
    mw->statusBar()->showMessage("Ready");
    mw->statusBar()->addPermanentWidget(new QLabel("Line 1, Col 1"));
    mw->setFixedHeight(190);
    v->addWidget(mw);

    // tree branches, dials, toolbox
    auto *row = new QHBoxLayout;
    auto *tree = new QTreeWidget; tree->setHeaderHidden(true);
    for (int i = 0; i < 3; ++i) {
        auto *top = new QTreeWidgetItem(tree, {QString("Folder %1").arg(i + 1)});
        new QTreeWidgetItem(top, {"File a"}); auto *sub = new QTreeWidgetItem(top, {"Sub folder"});
        new QTreeWidgetItem(sub, {"Deep file"});
        top->setExpanded(i != 1); sub->setExpanded(i == 0);
    }
    tree->setMaximumHeight(140);
    row->addWidget(tree);
    auto *dials = new QVBoxLayout;
    auto *d1 = new QDial; d1->setNotchesVisible(true); d1->setValue(30); d1->setFixedSize(64, 64);
    auto *d2 = new QDial; d2->setWrapping(true); d2->setValue(70); d2->setFixedSize(64, 64);
    auto *d3 = new QDial; d3->setEnabled(false); d3->setValue(50); d3->setFixedSize(64, 64);
    auto *dr = new QHBoxLayout; dr->addWidget(d1); dr->addWidget(d2); dr->addWidget(d3);
    dials->addLayout(dr);
    row->addLayout(dials);
    auto *box = new QToolBox;
    box->addItem(new QLabel("page one"), "Page one"); box->addItem(new QLabel("page two"), "Page two");
    box->addItem(new QLabel("page three"), "Page three");
    box->setMaximumHeight(140);
    row->addWidget(box);
    v->addLayout(row);

    // directly painted: size grips, rubber band, menu scroller, vertical toolbar separator/handle
    {
        QPixmap pm(420, 60);
        pm.fill(w->palette().window().color());
        QPainter pt(&pm);
        const Qt::Corner corners[] = {Qt::BottomRightCorner, Qt::BottomLeftCorner, Qt::TopRightCorner, Qt::TopLeftCorner};
        for (int i = 0; i < 4; ++i) {
            QStyleOptionSizeGrip g; g.initFrom(w); g.rect = QRect(8 + i * 24, 8, 16, 16); g.corner = corners[i];
            w->style()->drawControl(QStyle::CE_SizeGrip, &g, &pt, w);
        }
        QStyleOptionRubberBand rb; rb.initFrom(w); rb.rect = QRect(110, 6, 90, 40); rb.shape = QRubberBand::Rectangle;
        w->style()->drawControl(QStyle::CE_RubberBand, &rb, &pt, w);
        for (int i = 0; i < 2; ++i) {
            QStyleOptionMenuItem sc; sc.initFrom(w); sc.rect = QRect(215, 6 + i * 26, 90, 18);
            sc.menuItemType = QStyleOptionMenuItem::Scroller;
            sc.state = QStyle::State_Enabled | (i ? QStyle::State_DownArrow : QStyle::State_None);
            w->style()->drawControl(QStyle::CE_MenuScroller, &sc, &pt, w);
        }
        for (int i = 0; i < 2; ++i) {
            QStyleOption h; h.initFrom(w); h.rect = QRect(320 + i * 40, 6, 8, 44);
            h.state = QStyle::State_Enabled | QStyle::State_Horizontal;
            w->style()->drawPrimitive(i ? QStyle::PE_IndicatorToolBarSeparator : QStyle::PE_IndicatorToolBarHandle, &h, &pt, w);
        }
        pt.end();
        auto *l = new QLabel; l->setPixmap(pm); v->addWidget(l, 0, Qt::AlignLeft);
    }

    // menu with a section header
    auto *menu = new QMenu;
    menu->addSection("Recent files");
    menu->addAction("report.odt"); menu->addAction("notes.txt");
    menu->addSection("Tools");
    menu->addAction("Settings...");
    menu->popup(QPoint(0, 0));
    auto *lab = new QLabel; lab->setPixmap(menu->grab());
    menu->hide();
    v->addWidget(lab, 0, Qt::AlignLeft);
    return w;
}

static QWidget *gFocusList = nullptr;
static QWidget *gFocusEdit = nullptr;

static QWidget *framesViewsPage()
{
    auto *w = new QWidget;
    auto *v = new QVBoxLayout(w);

    auto *row = new QHBoxLayout;
    auto mk = [&](const char *title, bool checkable, bool checked, bool flat) {
        auto *g = new QGroupBox(title);
        g->setCheckable(checkable); g->setChecked(checked); g->setFlat(flat);
        auto *l = new QVBoxLayout(g);
        l->addWidget(new QCheckBox("Option"));
        l->addWidget(new QLabel("Contents"));
        row->addWidget(g);
    };
    mk("Plain", false, true, false);
    mk("Checkable", true, true, false);
    mk("Unchecked", true, false, false);
    mk("Flat", false, true, true);
    v->addLayout(row);

    auto *views = new QHBoxLayout;
    auto *list = new QListWidget; list->setAlternatingRowColors(true);
    for (int i = 0; i < 6; ++i) list->addItem(QString("List item %1").arg(i + 1));
    list->item(1)->setSelected(true); list->item(2)->setSelected(true);
    list->setMaximumHeight(150);
    gFocusList = list;                     // focused after show: active selection look; other views show the unfocused look
    views->addWidget(list);

    auto *table = new QTableWidget(5, 4);
    table->setHorizontalHeaderLabels({"Name", "Size", "Type", "Date"});
    for (int r = 0; r < 5; ++r) for (int c = 0; c < 4; ++c) table->setItem(r, c, new QTableWidgetItem(QString("r%1 c%2").arg(r).arg(c)));
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->selectRow(2);
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setSortIndicatorShown(true);
    table->setMaximumHeight(170);
    views->addWidget(table, 2);
    v->addLayout(views);

    auto *tree = new QTreeWidget; tree->setHeaderLabels({"Tree", "Value"});
    auto *a = new QTreeWidgetItem(tree, {"Parent", "1"});
    new QTreeWidgetItem(a, {"Child A", "2"}); auto *b = new QTreeWidgetItem(a, {"Child B", "3"});
    a->setExpanded(true); b->setSelected(true);
    tree->setMaximumHeight(110);
    v->addWidget(tree);

    // item states painted directly: normal / hover / selected / selected+hover / inactive selected / disabled
    {
        const char *names[] = {"Normal", "Hover", "Selected", "Sel+Hover", "Inactive", "Disabled"};
        QPixmap pm(560, 28);
        pm.fill(w->palette().base().color());
        QPainter pt(&pm);
        for (int i = 0; i < 6; ++i) {
            QStyleOptionViewItem it;
            it.initFrom(w);
            it.rect = QRect(4 + i * 92, 2, 88, 24);
            it.palette = w->palette();
            it.state = QStyle::State_Enabled | QStyle::State_Active;
            if (i == 1 || i == 3) it.state |= QStyle::State_MouseOver;
            if (i == 2 || i == 3 || i == 4) it.state |= QStyle::State_Selected;
            if (i == 4) it.state &= ~QStyle::State_Active;
            if (i == 5) it.state &= ~QStyle::State_Enabled;
            it.viewItemPosition = QStyleOptionViewItem::OnlyOne;
            w->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &it, &pt, w);
            it.rect.adjust(8, 0, 0, 0);
            pt.setPen(i == 2 || i == 3 || i == 4 ? w->palette().highlightedText().color() : w->palette().text().color());
            pt.drawText(it.rect, Qt::AlignVCenter, names[i]);
        }
        pt.end();
        auto *l = new QLabel; l->setPixmap(pm); v->addWidget(l, 0, Qt::AlignLeft);
    }

    auto *edit = new QTextEdit; edit->setPlainText("QTextEdit with PE_Frame border"); edit->setMaximumHeight(50);
    v->addWidget(edit);

    auto *split = new QSplitter(Qt::Horizontal);
    split->addWidget(new QTextEdit("left")); split->addWidget(new QTextEdit("right"));
    split->setMaximumHeight(50);
    v->addWidget(split);

    // splitter handle states + tooltip drawn directly
    {
        QPixmap pm(380, 56);
        pm.fill(w->palette().window().color());
        QPainter pt(&pm);
        for (int i = 0; i < 3; ++i) {
            QStyleOption so; so.initFrom(w); so.rect = QRect(10 + i * 30, 4, 6, 48);
            so.state = QStyle::State_Enabled | QStyle::State_Horizontal;
            if (i == 1) so.state |= QStyle::State_MouseOver;
            if (i == 2) so.state |= QStyle::State_Sunken;
            w->style()->drawControl(QStyle::CE_Splitter, &so, &pt, w);
        }
        for (int i = 0; i < 2; ++i) {
            QStyleOption so; so.initFrom(w); so.rect = QRect(110, 4 + i * 26, 100, 6);
            so.state = QStyle::State_Enabled | (i ? QStyle::State_MouseOver : QStyle::State_None);
            w->style()->drawControl(QStyle::CE_Splitter, &so, &pt, w);
        }
        QStyleOption tip; tip.initFrom(w); tip.rect = QRect(230, 10, 130, 24);
        tip.palette = w->palette();
        w->style()->drawPrimitive(QStyle::PE_PanelTipLabel, &tip, &pt, w);
        pt.setPen(w->palette().toolTipText().color());
        pt.drawText(tip.rect.adjusted(6, 0, 0, 0), Qt::AlignVCenter, "A tooltip");
        pt.end();
        auto *l = new QLabel; l->setPixmap(pm); v->addWidget(l, 0, Qt::AlignLeft);
    }

    auto *mw = new QMainWindow;
    mw->setCentralWidget(new QTextEdit("central"));
    auto *dock = new QDockWidget("Dock title", mw);
    dock->setWidget(new QLabel("dock content"));
    mw->addDockWidget(Qt::LeftDockWidgetArea, dock);
    mw->setMinimumHeight(110); mw->setMaximumHeight(110);
    v->addWidget(mw);
    return w;
}

static QWidget *tabsMenusPage()
{
    auto *w = new QWidget;
    auto *v = new QVBoxLayout(w);

    auto *mb = new QMenuBar;
    mb->addMenu("&File"); auto *edit = mb->addMenu("&Edit"); mb->addMenu("&View");
    auto *dis = mb->addMenu("&Disabled"); dis->setEnabled(false);
    edit->addAction("Cut");
    v->addWidget(mb);
    // active/pressed menubar items: setActiveAction() would pop the submenu up, so paint them directly
    {
        QPixmap pm(260, 26);
        pm.fill(Qt::transparent);
        QPainter pt(&pm);
        QStyleOptionMenuItem bar;
        bar.initFrom(w); bar.rect = pm.rect();
        w->style()->drawControl(QStyle::CE_MenuBarEmptyArea, &bar, &pt, w);
        const char *names[] = {"&File", "&Edit", "&View", "&Gone"};
        for (int i = 0; i < 4; ++i) {
            QStyleOptionMenuItem it;
            it.initFrom(w);
            it.rect = QRect(4 + i * 62, 0, 58, 26);
            it.text = names[i];
            it.menuItemType = QStyleOptionMenuItem::Normal;
            it.state = QStyle::State_Enabled | (i == 1 ? QStyle::State_Selected : QStyle::State_None)
                       | (i == 2 ? (QStyle::State_Selected | QStyle::State_Sunken) : QStyle::State_None);
            if (i == 3) it.state = QStyle::State_None;     // disabled
            w->style()->drawControl(QStyle::CE_MenuBarItem, &it, &pt, w);
        }
        pt.end();
        auto *l = new QLabel; l->setPixmap(pm); v->addWidget(l, 0, Qt::AlignLeft);
    }

    auto tabsFor = [](QTabWidget::TabPosition pos, const char *title) {
        auto *tw = new QTabWidget;
        tw->setTabPosition(pos);
        auto *l = new QLabel(title); l->setMargin(10);
        tw->addTab(l, "General");
        tw->addTab(new QLabel("two"), "Advanced");
        tw->addTab(new QLabel("three"), "Disabled");
        tw->setTabEnabled(2, false);
        tw->addTab(new QLabel("four"), "About");
        tw->setCurrentIndex(0);
        tw->setMinimumHeight(110);
        return tw;
    };
    v->addWidget(tabsFor(QTabWidget::North, "North"));
    v->addWidget(tabsFor(QTabWidget::South, "South"));
    auto *h = new QHBoxLayout;
    h->addWidget(tabsFor(QTabWidget::West, "West"));
    h->addWidget(tabsFor(QTabWidget::East, "East"));
    v->addLayout(h);
    auto *bar = new QTabBar; bar->addTab("Standalone"); bar->addTab("Tab bar"); bar->addTab("Doc");
    v->addWidget(bar);

    // popup menu rendered to a pixmap (works offscreen)
    auto *menu = new QMenu;   // top-level: a parent without a window handle crashes popup()
    auto *style = w->style();
    menu->addAction(style->standardIcon(QStyle::SP_DialogOpenButton), "&Open...", QKeySequence::Open);
    menu->addAction(style->standardIcon(QStyle::SP_DialogSaveButton), "&Save", QKeySequence::Save);
    auto *a = menu->addAction("Checkable"); a->setCheckable(true); a->setChecked(true);
    auto *grp = new QActionGroup(menu);
    auto *r1 = menu->addAction("Radio A"); r1->setCheckable(true); r1->setChecked(true); grp->addAction(r1);
    auto *r2 = menu->addAction("Radio B"); r2->setCheckable(true); grp->addAction(r2);
    menu->addSeparator();
    menu->addMenu("Submenu")->addAction("Child");
    auto *off = menu->addAction("Disabled item"); off->setEnabled(false);
    auto *act = menu->addAction("Hovered item", QKeySequence("Ctrl+H"));
    menu->setActiveAction(act);
    menu->popup(QPoint(0, 0));          // must be shown for a screen/window handle (offscreen is fine)
    auto *lab = new QLabel; lab->setPixmap(menu->grab());
    menu->hide();
    v->addWidget(lab, 0, Qt::AlignLeft);
    return w;
}

static QWidget *remainingPage()
{
    auto *w = new QWidget;
    auto *v = new QVBoxLayout(w);

    auto *row = new QHBoxLayout;
    auto *mdi = new QMdiArea;
    mdi->setMaximumHeight(170);
    auto *sub = mdi->addSubWindow(new QLabel("MDI subwindow"));
    sub->resize(300, 120);
    sub->show();
    row->addWidget(mdi, 2);
    auto *cal = new QCalendarWidget;
    cal->setMaximumHeight(170);
    row->addWidget(cal);
    v->addLayout(row);

    auto *edit = new QTextEdit;
    edit->setPlainText("focused QTextEdit: PE_Frame focus glow");
    edit->setMaximumHeight(50);
    gFocusEdit = edit;               // focused after show: frame paints the glow
    v->addWidget(edit);

    // directly painted: title bar, toolbar extension, menu tear-off
    {
        QPixmap pm(560, 84);
        pm.fill(w->palette().window().color());
        QPainter pt(&pm);
        QStyleOptionTitleBar tb;
        tb.initFrom(w);
        tb.rect = QRect(4, 4, 380, 22);
        tb.text = "AeroQt title bar";
        tb.subControls = QStyle::SC_TitleBarLabel | QStyle::SC_TitleBarMinButton
                         | QStyle::SC_TitleBarMaxButton | QStyle::SC_TitleBarCloseButton | QStyle::SC_TitleBarSysMenu;
        tb.state = QStyle::State_Enabled | QStyle::State_Active;
        w->style()->drawComplexControl(QStyle::CC_TitleBar, &tb, &pt, w);
        for (int i = 0; i < 2; ++i) {  // toolbar extension icons (standard pixmaps, not a primitive)
            QStyleOption ext;
            ext.initFrom(w);
            const QRect r(400 + i * 60, 4, 22, 22);
            const auto sp = i ? QStyle::SP_ToolBarVerticalExtensionButton : QStyle::SP_ToolBarHorizontalExtensionButton;
            pt.drawPixmap(r.topLeft(), w->style()->standardPixmap(sp, &ext));
        }
        QStyleOption to;
        to.initFrom(w);
        to.rect = QRect(4, 36, 380, 16);
        w->style()->drawControl(QStyle::CE_MenuTearoff, &to, &pt, w);
        pt.setPen(w->palette().text().color());
        pt.drawText(QRect(4, 56, 380, 20), Qt::AlignVCenter, "tear-off strip above");
        pt.end();
        auto *l = new QLabel;
        l->setPixmap(pm);
        v->addWidget(l, 0, Qt::AlignLeft);
    }

    // wizard + progress dialog rendered to pixmaps (top-level, no parent)
    {
        auto *wiz = new QWizard;
        auto *page = new QWizardPage;
        auto *lay = new QVBoxLayout(page);
        lay->addWidget(new QLabel("Wizard page"));
        lay->addWidget(new QLineEdit("Aero"));
        wiz->addPage(page);
        wiz->resize(360, 220);
        wiz->show();
        auto *lab = new QLabel;
        lab->setPixmap(wiz->grab());
        wiz->hide();
        delete wiz;
        v->addWidget(lab, 0, Qt::AlignLeft);
    }
    {
        auto *pd = new QProgressDialog("Copying files...", "Cancel", 0, 100);
        pd->setValue(40);
        pd->resize(340, 110);
        pd->show();
        auto *lab = new QLabel;
        lab->setPixmap(pd->grab());
        pd->hide();
        delete pd;
        v->addWidget(lab, 0, Qt::AlignLeft);
    }
    return w;
}

int main(int argc, char **argv)
{
    // Parse args first: --theme= must be in the env before AeroStyle is constructed.
    int page = 0;   // 0 = all
    QString out = QStringLiteral("preview.png");
    for (int i = 1; i < argc; ++i) {
        const QString a = QString::fromLocal8Bit(argv[i]);
        if (a.startsWith("--page=")) page = a.mid(7).toInt();
        else if (a.startsWith("--theme=")) qputenv("AEROQT_THEME_NAME", a.mid(8).toLocal8Bit());
        else if (!a.startsWith("--")) out = a;
    }

    QApplication app(argc, argv);
    app.setStyle(new AeroStyle);
    const bool dark = app.arguments().contains(QStringLiteral("--dark"));
    setPalette(app, dark);

    QWidget root;
    auto *lay = new QHBoxLayout(&root);
    if (page == 0 || page == 1) lay->addWidget(basicsPage());
    if (page == 0 || page == 2) lay->addWidget(rangePage());
    if (page == 0 || page == 3) lay->addWidget(tabsMenusPage());
    if (page == 0 || page == 4) lay->addWidget(framesViewsPage());
    if (page == 0 || page == 5) lay->addWidget(miscPage());
    if (page == 0 || page == 6) lay->addWidget(remainingPage());
    root.resize(root.sizeHint());
    root.show();
    root.activateWindow();
    QApplication::setActiveWindow(&root);      // views paint 'active' selection colors
    if (gFocusList) gFocusList->setFocus(Qt::OtherFocusReason);
    if (gFocusEdit) gFocusEdit->setFocus(Qt::OtherFocusReason);
    QApplication::processEvents();
    root.grab().save(out);
    return 0;
}
