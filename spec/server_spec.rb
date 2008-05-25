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
    
  it "should POST in chunks" do
    POST("/", %w(oh hi there !)) do
      status.should == 200
      headers['Content-Type'].should == 'text/html'
      headers['Content-Length'].should == '10'
      body.should == 'ohhithere!'
    end
  end
  
  it "should handle big body (stored in memory)" do
    data = [rand_data(1024)] * (80 + 31)
    expected_size = data.join.size
    
    POST("/", data) do
      status.should == 200
      headers['Content-Length'].should == expected_size.to_s
      body.size.should == expected_size
      body.should == data.join
    end
    
    @app.env['rack.input'].class.should == StringIO
  end
  
  it "should handle very big body (stored to tempfile)" do
    data = [rand_data(1024)] * 1024
    expected_size = data.join.size
    
    POST("/", data) do
      status.should == 200
      headers['Content-Length'].should == expected_size.to_s
      body.size.should == expected_size
      body.should == data.join
    end
    
    @app.env['rack.input'].class.should == File
  end
end