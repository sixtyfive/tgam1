function message_received(topic, data)
  global eeg_data;
  
  dim = size(eeg_data);
  len = dim(2);
  
  if (len >= 1000)
    eeg_data(1) = [];
  end
  eeg_data = [eeg_data, data];
end