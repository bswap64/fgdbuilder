#pragma once
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class FGDHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit FGDHighlighter(QTextDocument *parent = nullptr);
protected:
    void highlightBlock(const QString &text) override;
private:
    struct Rule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QList<Rule> m_rules;
    QRegularExpression m_commentPattern;
    QTextCharFormat m_commentFormat;
};
