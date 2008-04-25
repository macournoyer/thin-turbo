require File.dirname(__FILE__) + '/spec_helper'

describe Thin::Backends::Turbo, "streaming" do
  serve StreamedApp.new
  
  it "should GET 10 chunks of 1024 bytes" do
    GET("/?chunks=10&size=1024") do
      status.should == 200
      body.size.should == 10 * 1024
    end
  end
end
