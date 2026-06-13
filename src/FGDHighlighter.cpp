#include "FGDHighlighter.h"

FGDHighlighter::FGDHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
    QTextCharFormat kwFormat;
    kwFormat.setForeground(QColor("#569cd6"));
    kwFormat.setFontWeight(QFont::Bold);
    Rule r;
    r.pattern = QRegularExpression("@(PointClass|SolidClass|NPCClass|BaseClass|FilterClass|KeyFrameClass|MoveClass|ExtendClass|include|mapsize|version|MaterialExclusion|AutoVisGroup)\\b");
    r.format = kwFormat;
    m_rules.append(r);

    QTextCharFormat propFormat;
    propFormat.setForeground(QColor("#4ec9b0"));
    r.pattern = QRegularExpression("\\b(base|color|size|studio|studioprop|iconsprite|sphere|line|vecline|axis|wirebox|origin|lightprop|decal|overlay|sprite|sweptplayerhull|instance|animator|keyframe|worldtext|halfgridsnap)\\b");
    r.format = propFormat;
    m_rules.append(r);

    QTextCharFormat typeFormat;
    typeFormat.setForeground(QColor("#ce9178"));
    r.pattern = QRegularExpression("\\(\\s*(string|integer|float|boolean|choices|flags|angle|color255|color1|void|bool|vector|studio|material|sound|sprite|target_destination|target_source|target_name_or_class|node_id|node_dest|filterclass|npcclass|pointentityclass|scene|script|scriptlist|sidelist|particlesystem|instance_file|instance_parm|instance_variable|origin|decal|ehandle|angle_negative_pitch|sky|soundscape|axis|vecline)\\s*\\)");
    r.format = typeFormat;
    m_rules.append(r);

    QTextCharFormat ioFormat;
    ioFormat.setForeground(QColor("#dcdcaa"));
    r.pattern = QRegularExpression("\\b(input|output)\\b");
    r.format = ioFormat;
    m_rules.append(r);

    QTextCharFormat modFormat;
    modFormat.setForeground(QColor("#c586c0"));
    r.pattern = QRegularExpression("\\b(readonly|report)\\b");
    r.format = modFormat;
    m_rules.append(r);

    QTextCharFormat numberFormat;
    numberFormat.setForeground(QColor("#b5cea8"));
    r.pattern = QRegularExpression("\\b-?\\d+(\\.\\d+)?\\b");
    r.format = numberFormat;
    m_rules.append(r);

    QTextCharFormat strFormat;
    strFormat.setForeground(QColor("#ce9178"));
    r.pattern = QRegularExpression("\"[^\"]*\"");
    r.format = strFormat;
    m_rules.append(r);

    m_commentFormat.setForeground(QColor("#6a9955"));
    m_commentPattern = QRegularExpression("//[^\n]*");
}

void FGDHighlighter::highlightBlock(const QString &text) {
    int commentStart = -1;
    QRegularExpressionMatch cm = m_commentPattern.match(text);
    if (cm.hasMatch()) commentStart = cm.capturedStart();

    for (const Rule &rule : m_rules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch m = it.next();
            int start = m.capturedStart();
            int len   = m.capturedLength();
            if (commentStart >= 0 && start >= commentStart) continue;
            if (commentStart >= 0 && start < commentStart && start + len > commentStart)
                len = commentStart - start;
            if (len > 0) setFormat(start, len, rule.format);
        }
    }

    if (commentStart >= 0)
        setFormat(commentStart, text.length() - commentStart, m_commentFormat);
}
