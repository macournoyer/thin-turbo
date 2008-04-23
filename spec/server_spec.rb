require File.dirname(__FILE__) + '/spec_helper'

describe Thin::Backends::Turbo, "server" do
  serve EchoApp.new
  
  it do
    GET("/") do
      status.should == 200
      headers['Content-Type'].should == 'text/html'
      headers['Content-Length'].should == '1'
      body.should == '/'
    end
  end
  
  it do
    POST("/", "hi") do
      status.should == 200
      headers['Content-Type'].should == 'text/html'
      headers['Content-Length'].should == '2'
      body.should == 'hi'
    end
  end
    
  it "should handle big body (stored in memory)" do
    data = 'X' * 1024 * (80 + 31)
    
    POST("/", data) do
      status.should == 200
      headers['Content-Length'].should == data.size.to_s
      body.should == data
    end
  end

  xit "should handle very big body (stored to tempfile)" do
    data = 'X' * 1024 ** 2
    
    POST("/", data) do
      status.should == 200
      headers['Content-Length'].should == data.size.to_s
      body.should == data
    end
  end
end