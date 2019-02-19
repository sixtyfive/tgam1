global eeg_data;
eeg_data = [];

broker = mqtt('tcp://rennsemmel');
topic = subscribe(broker, 'MindFlex/data');

topic.Callback = @message_received;