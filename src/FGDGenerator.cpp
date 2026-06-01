#include "FGDGenerator.h"
#include <QStringList>

QString FGDGenerator::escapeDescription(const QString &desc) {
    QString r = desc;
    r.replace("\"", "'");
    return r;
}

QString FGDGenerator::generateKeyvalue(const FGDKeyvalue &kv) {
    QString line;
    if (kv.type == "flags") {
        line = "\t" + (kv.name.isEmpty() ? QString("spawnflags") : kv.name) + "(flags) =\n\t[\n";
        for (int i = 0; i < kv.flagItems.size(); ++i) {
            auto &f = kv.flagItems[i];
            QString desc = (i < kv.flagDescriptions.size()) ? kv.flagDescriptions[i] : QString();
            if (!desc.isEmpty()) {
                line += QString("\t\t%1 : \"%2\" : %3 : \"%4\"\n")
                    .arg(f.first)
                    .arg(escapeDescription(f.second.first))
                    .arg(f.second.second)
                    .arg(escapeDescription(desc));
            } else {
                line += QString("\t\t%1 : \"%2\" : %3\n")
                    .arg(f.first)
                    .arg(escapeDescription(f.second.first))
                    .arg(f.second.second);
            }
        }
        line += "\t]\n";
        return line;
    }
    line = "\t" + kv.name + "(" + kv.type + ")";
    if (kv.readOnly) line += " readonly";
    if (kv.report)   line += " report";
    if (!kv.displayName.isEmpty() || !kv.defaultValue.isEmpty() || !kv.description.isEmpty()) {
        line += " : \"" + escapeDescription(kv.displayName) + "\"";
        if (!kv.defaultValue.isEmpty() || !kv.description.isEmpty()) {
            if (kv.type == "string" || kv.type == "float") {
                line += " : \"" + kv.defaultValue + "\"";
            } else {
                line += kv.defaultValue.isEmpty() ? " : " : (" : " + kv.defaultValue);
            }
            if (!kv.description.isEmpty()) {
                line += " : \"" + escapeDescription(kv.description) + "\"";
            }
        }
    }
    if (kv.type == "choices") {
        line += " =\n\t[\n";
        for (int i = 0; i < kv.choiceItems.size(); ++i) {
            auto &c = kv.choiceItems[i];
            QString desc = (i < kv.choiceDescriptions.size()) ? kv.choiceDescriptions[i].second : QString();
            if (!desc.isEmpty()) {
                line += "\t\t" + c.first + " : \"" + escapeDescription(c.second) + "\" : \"" + escapeDescription(desc) + "\"\n";
            } else {
                line += "\t\t" + c.first + " : \"" + escapeDescription(c.second) + "\"\n";
            }
        }
        line += "\t]";
    }
    return line + "\n";
}

QString FGDGenerator::generateIO(const FGDIO &io) {
    QString line = "\t";
    line += io.isInput ? "input " : "output ";
    line += io.name + "(" + io.type + ")";
    if (!io.description.isEmpty()) {
        line += " : \"" + escapeDescription(io.description) + "\"";
    }
    return line + "\n";
}

QString FGDGenerator::generateEntity(const FGDEntity &entity) {
    QString out;
    out += classTypeToString(entity.classType);
    if (!entity.bases.isEmpty())
        out += " base(" + entity.bases.join(", ") + ")";
    if (!entity.color.isEmpty())
        out += " color(" + entity.color + ")";
    if (!entity.size.isEmpty())
        out += " size(" + entity.size + ")";
    if (!entity.studioModel.isEmpty())
        out += " studio(\"" + entity.studioModel + "\")";
    if (!entity.studioprop.isEmpty())
        out += " studioprop(\"" + entity.studioprop + "\")";
    if (!entity.iconSprite.isEmpty())
        out += " iconsprite(\"" + entity.iconSprite + "\")";
    if (!entity.sphere.isEmpty())
        out += " sphere(" + entity.sphere + ")";
    if (!entity.line.isEmpty())
        out += " line(" + entity.line + ")";
    if (!entity.vecline.isEmpty())
        out += " vecline(" + entity.vecline + ")";
    if (!entity.axis.isEmpty())
        out += " axis(" + entity.axis + ")";
    if (!entity.wirebox.isEmpty())
        out += " wirebox(" + entity.wirebox + ")";
    if (entity.decal)          out += " decal()";
    if (entity.overlay)        out += " overlay()";
    if (entity.sprite)         out += " sprite()";
    if (entity.sweptplayerhull) out += " sweptplayerhull()";
    if (entity.instance_)      out += " instance()";
    if (entity.animator)       out += " animator()";
    if (entity.keyframe_)      out += " keyframe()";
    if (entity.worldtext)      out += " worldtext()";
    if (entity.halfGridSnap)   out += " halfgridsnap";

    out += " = " + entity.className;
    if (!entity.description.isEmpty()) {
        QString desc = escapeDescription(entity.description);
        if (desc.length() > 120) {
            QStringList parts;
            int pos = 0;
            while (pos < desc.length()) {
                parts << desc.mid(pos, 120);
                pos += 120;
            }
            out += " :\n\t\"" + parts.join("\" +\n\t\"") + "\"";
        } else {
            out += " : \"" + desc + "\"";
        }
    }
    out += "\n[\n";
    for (auto &kv : entity.keyvalues)
        out += generateKeyvalue(kv);
    bool hasInputs = false, hasOutputs = false;
    for (auto &io : entity.ios) {
        if (io.isInput)  hasInputs  = true;
        else             hasOutputs = true;
    }
    if (hasInputs) {
        out += "\n\t// Inputs\n";
        for (auto &io : entity.ios)
            if (io.isInput) out += generateIO(io);
    }
    if (hasOutputs) {
        out += "\n\t// Outputs\n";
        for (auto &io : entity.ios)
            if (!io.isInput) out += generateIO(io);
    }
    out += "]\n\n";
    return out;
}

static void generateAutoVisGroupChildren(QString &out, const QList<FGDAutoVisGroupChild> &children, int depth) {
    QString indent(depth, '\t');
    for (auto &child : children) {
        out += indent + "\"" + child.name + "\"\n" + indent + "[\n";
        for (auto &e : child.entities)
            out += indent + "\t\"" + e + "\"\n";
        generateAutoVisGroupChildren(out, child.children, depth + 1);
        out += indent + "]\n";
    }
}

QString FGDGenerator::generate(const FGDProject &project) {
    QString out;
    if (!project.headerComment.isEmpty()) {
        for (auto &l : project.headerComment.split("\n"))
            out += "// " + l + "\n";
        out += "\n";
    }
    for (auto &inc : project.includes)
        out += "@include \"" + inc + "\"\n";
    if (!project.includes.isEmpty()) out += "\n";
    if (project.useMapsSize)
        out += QString("@mapsize(%1, %2)\n\n").arg(project.mapsizeMin).arg(project.mapsizeMax);
    if (!project.materialExclusions.isEmpty()) {
        out += "@MaterialExclusion\n[\n";
        for (auto &m : project.materialExclusions)
            out += "\t\"" + m + "\"\n";
        out += "]\n\n";
    }
    for (auto &avg : project.autoVisGroups) {
        out += "@AutoVisGroup = \"" + avg.parent + "\"\n[\n";
        generateAutoVisGroupChildren(out, avg.children, 1);
        out += "]\n\n";
    }
    for (auto &entity : project.entities)
        out += generateEntity(entity);
    return out;
}
