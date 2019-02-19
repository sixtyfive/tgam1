function message_received(~, data)
  global eeg_data;
  
  if (size(eeg_data) > 10000)
    eeg_data(1) = [];
  end
  eeg_data = [eeg_data, data];
end