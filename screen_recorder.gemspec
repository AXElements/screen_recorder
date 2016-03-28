require './lib/screen_recorder/version'

Gem::Specification.new do |s|
  s.name     = 'screen_recorder'
  s.version  = ScreenRecorder::VERSION

  s.summary     = 'A class that allows recording your screen in OS X'
  s.description = <<-EOS
screen_recorder is a wrapper around some of the OS X AVFoundation framework.

Originally extracted from the AXElements project.
  EOS

  s.authors     = ['Mark Rada']
  s.email       = 'markrada26@gmail.com'
  s.homepage    = 'http://github.com/AXElements/screen_recorder'
  s.licenses    = ['BSD-3-Clause']
  s.has_rdoc    = 'yard'

  s.extensions  = ['ext/screen_recorder/extconf.rb']
  s.files       = Dir.glob('lib/**/*.rb') +
                  Dir.glob('ext/**/*.{c,h,rb,m}') +
                  [
                   'Rakefile',
                   'README.markdown',
                   'History.markdown',
                   '.yardopts'
                  ]
end
