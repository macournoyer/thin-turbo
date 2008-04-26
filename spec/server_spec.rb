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
    
  it "POST in chunks" do
    POST("/", %w(oh hi there !)) do
      status.should == 200
      headers['Content-Type'].should == 'text/html'
      headers['Content-Length'].should == '10'
      body.should == 'ohhithere!'
    end
  end
    
  it "should handle big body (stored in memory)" do
    data = ['X' * 1024] * (80 + 31)
    
    POST("/", data) do
      status.should == 200
      headers['Content-Length'].should == (1024 * (80 + 31)).to_s
      body.should == data.join
    end
    
    @app.env['rack.input'].class.should == StringIO
  end
  
  # FIXME fail cause response body is stored in a tmpfile (body is a String...)
  # maybe connection.write_buffer needs to be changed to a chain of Ruby strings
  xit "should handle very big body (stored to tempfile)" do
    data = ['X' * 1024] * (80 + 33)
    
    POST("/", data) do
      status.should == 200
      headers['Content-Length'].should == (1024 ** 2).to_s
      body.should == data.join
    end
    
    @app.env['rack.input'].class.should == File
  end
end