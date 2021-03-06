#include "programsmodel.h"

#include "channel.h"
#include "database.h"
#include "fetcher.h"
#include "program.h"
#include "programfactory.h"
#include "types.h"

#include <QDebug>

ProgramsModel::ProgramsModel(Channel *channel, ProgramFactory &programFactory)
    : QAbstractListModel(channel)
    , m_channel(channel)
    , m_programFactory(programFactory)
{
    connect(&Fetcher::instance(), &Fetcher::channelUpdated, this, [this](const ChannelId &id) {
        if (m_channel->id() == id.value()) {
            beginResetModel();
            m_programFactory.load(ChannelId(m_channel->id()));
            for (auto &program : m_programs) {
                delete program;
            }
            m_programs.clear();
            endResetModel();
        }
    });
}

ProgramsModel::~ProgramsModel()
{
    qDeleteAll(m_programs);
}

QVariant ProgramsModel::data(const QModelIndex &index, int role) const
{
    if (role != 0) {
        return QVariant();
    }
    if (m_programs[index.row()] == nullptr) {
        loadProgram(index.row());
    }
    return QVariant::fromValue(m_programs[index.row()]);
}

QHash<int, QByteArray> ProgramsModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames[0] = "program";
    return roleNames;
}

int ProgramsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_programFactory.count(ChannelId(m_channel->id()));
}

void ProgramsModel::loadProgram(int index) const
{
    Program *program = m_programFactory.create(ChannelId(m_channel->id()), index);

    if (program) {
        // avoid gaps/overlapping in the program (causes not aligned times in table)
        if (m_programs.contains(index - 1) && m_programs[index - 1]->stop() != program->start()) {
            program->setStart(m_programs[index - 1]->stop());
        }
        m_programs[index] = program;
    }
}

Channel *ProgramsModel::channel() const
{
    return m_channel;
}
