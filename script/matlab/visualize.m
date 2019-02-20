global eeg_data;
eeg_data = [];

broker = mqtt('tcp://SERVER_ADDRESS');
topic = subscribe(broker, 'MindFlex/data');

topic.Callback = @message_received;
