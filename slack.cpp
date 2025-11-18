#include <stdio.h>
#include "include/slacking/slacking.hpp"
#include "include/slacking/json.hpp"

#include <fstream>

#include <iostream>

int main() {
    const char* token = std::getenv("SLACK_TOKEN");
    auto& slack = slack::create(token); // "xxx-xxx-xxx-xxx" is your Slack API token
    slack.chat.channel = "#temperature-control"; // set a default channel

    slack.chat.postMessage("Hello there!");
    return 0;
}