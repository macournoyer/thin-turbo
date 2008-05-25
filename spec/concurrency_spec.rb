require File.dirname(__FILE__) + '/spec_helper'

describe Thin::Backends::Turbo, "concurrency" do
  serve EchoApp.new
  
  it "should handle concurrency" do    
    threads = []
    10.times do
      threads << Thread.new do
        data = rand_data(1024, 1024 ** 2)
        
        POST("/", data) do
          status.should == 200
          headers['Content-Length'].should == data.size.to_s
          body.size.should == data.size
          body.should == data
        end
      end
    end
    
    threads.each { |t| t.join }
  end  
end