get Slack token from Zev, add:

export SLACK_TOKEN="token"

to your ~/.bashrc

source ~/.bashrc
sudo apt-get install libcurl4-openssl-dev
g++ slack.cpp -o slack -lcurl

compile and run