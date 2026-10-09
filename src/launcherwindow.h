#ifndef LAUNCHERWINDOW_H
#define LAUNCHERWINDOW_H

#include "uilanguage.h"

#include <QPoint>
#include <QWidget>

class QLabel;
class QMouseEvent;
class QPushButton;

class LauncherWindow final : public QWidget {
    Q_OBJECT
public:
    explicit LauncherWindow(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QPushButton *makeActionButton(const QString &title, const QString &subtitle,
                                  const QString &objectName);
    QPushButton *makeLanguageButton(const QString &text, UiLanguage language);
    void applyLanguage(UiLanguage language, bool persist = true);
    void updateLanguageButtonState();
    void updateScreenInfo();
    void openDvdWindow(bool fullScreen);
    void openRefreshRateGuide();

    UiLanguage m_language = UiLanguage::Chinese;
    QLabel *m_brandLabel = nullptr;
    QLabel *m_editionLabel = nullptr;
    QLabel *m_speechLabel = nullptr;
    QLabel *m_eyebrowLabel = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_introLabel = nullptr;
    QLabel *m_screenInfoLabel = nullptr;
    QLabel *m_creatorLabel = nullptr;
    QPushButton *m_windowedButton = nullptr;
    QPushButton *m_fullScreenButton = nullptr;
    QPushButton *m_refreshButton = nullptr;
    QPushButton *m_chineseButton = nullptr;
    QPushButton *m_englishButton = nullptr;
    QPushButton *m_japaneseButton = nullptr;
    QPoint m_dragOffset;
    bool m_dragging = false;
};

#endif
