#ifndef PROMPT_P_H
#define PROMPT_P_H

class QAuthPrompt::Private {
public:
    QAuthPrompt::Type type;
    QByteArray response;
    QString message;
    bool hidden;
};

#endif // PROMPT_P_H