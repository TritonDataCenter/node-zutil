{
  'targets': [
    {
      'target_name': 'zutil_bindings',
      'sources': [  './src/zone.cc', './src/zonecfg.cc', './src/zutil_bindings.cc' ],
      'libraries': [ '-lpthread', '-lzonecfg' ],
      'cflags_cc': [ '-Wno-write-strings' ],
      'cflags_cc!': [ '-fno-exceptions' ],
    }
  ]
}
