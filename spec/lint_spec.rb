require File.dirname(__FILE__) + '/spec_helper'

describe Thin::Backends::Turbo, "Lint middleware" do
  serve Rack::Lint.new(EchoApp.new)
  
  it "should validate GET" do
    GET("/path?query") do
      status.should == 200
    end
  end

  it "should validate POST" do
    POST("/path", "query=cool") do
      status.should == 200
    end
  end
end
