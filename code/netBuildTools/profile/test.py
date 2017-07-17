status = False
try:
    from past import autotranslate
    autotranslate(['snap'])
    import snap
    version = snap.Version
    i = snap.TInt(5)
    if i == 5:
        status = True
except Exception as e:
    print(e)
    pass

if status:
    print("SUCCESS, your version of Snap.py is %s" % (version))
else:
    print("*** ERROR, no working Snap.py was found on your computer")


