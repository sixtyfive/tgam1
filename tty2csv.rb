#!/usr/bin/env ruby

require 'serialport'
require 'date'
require 'csv'

filename = Time.now.strftime('mindflex-data-%d%m%Y%H%M.csv')
headers = %w{signal_strength raw attention meditation delta theta low_alpha high_alpha low_beta high_beta low_gamma high_gamma}

begin
  tty = SerialPort.new(ARGV[0] || '/dev/ttyUSB0', ARGV[1] ? ARGV[1].to_i : 115200, 8, 1, SerialPort::NONE)
rescue
  puts "Error reading from TTY. Is a device connected?"
  exit
end

puts "Reading..."

csv = CSV.open(filename, 'a', write_headers: true, headers: headers)
csv.sync = true

begin
  response = tty.readline("\r\n")
  begin
    datapoint = response.chomp.split(',')
    if datapoint.length == headers.length
      puts datapoint.join("\t")
      csv << datapoint
    end
  rescue Exception => e
    puts "Error writing to #{filename}: #{e}"
    csv.close
  end
end while true
