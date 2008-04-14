require File.dirname(__FILE__) + '/spec_helper'

describe Thin, "turbo backend" do
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
end