AddOption("--docs", action="store_true", dest="docs", default=False)
if GetOption("docs"):
    SConscript('doc/SConstruct')
else:
    SConscript('src/SConstruct', variant_dir='build', duplicate=0)
