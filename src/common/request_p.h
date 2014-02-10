#ifndef REQUEST_P_H
#define REQUEST_P_H

class QAuthRequest::Private {
public:
    QString info;
    QList<QAuthPrompt*> prompts;
};

#endif // REQUEST_P_H