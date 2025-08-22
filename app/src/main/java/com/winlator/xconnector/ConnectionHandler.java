package com.winlator.xconnector;

public interface ConnectionHandler {
    void handleConnectionShutdown(ConnectedClient client);

    void handleNewConnection(ConnectedClient client);
}