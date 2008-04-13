require File.dirname(__FILE__) + '/spec_helper'

module HttpMatchers
  
end

describe Thin, "turbo backend" do
  serve App.new
  
  it "should respond to GET" do
    response = request("GET / HTTP/1.1\r\nConnection: close\r\n\r\n")
    response.should == "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 1\r\n\r\n/"
  end
  
  it "should respond to POST" do
    response = request("POST / HTTP/1.1\r\nConnection: close\r\nContent-Length: 2\r\n\r\nhi")
    response.should == "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 3\r\n\r\n/hi"
  end
end