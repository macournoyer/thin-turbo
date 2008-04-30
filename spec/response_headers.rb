require File.dirname(__FILE__) + '/spec_helper'

describe Thin::Backends::Turbo, "response headers" do
  before do
    @@headers = {}
  end

  serve do |env|
    [200, @@headers, ""]
  end
      
  it "should handle simple header value" do
    @@headers['X-Cool'] = 'fo sho!'
    
    GET('/') do
      headers['X-Cool'].should == @@headers['X-Cool']
    end
  end
  
  it "should handle repeated header" do
    @@headers['X-Cool'] = "oh\naie"
    
    GET('/') do
      headers['X-Cool'].should == @@headers['X-Cool'].split("\n")
    end
  end
  
  it "should handle large header name" do
    @@headers['X' * (1024 ** 2)] = 'ouch!'
    
    GET('/') do
      headers.keys.first.size.should == (1024 ** 2)
    end
  end

  it "should handle large header value" do
    @@headers['X-Large'] = 'X' * (1024 ** 2)
    
    GET('/') do
      headers['X-Large'].size.should == (1024 ** 2)
    end
  end
end