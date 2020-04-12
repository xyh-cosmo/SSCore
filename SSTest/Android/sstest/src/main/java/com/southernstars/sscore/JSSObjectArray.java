package com.southernstars.sscore;

public class JSSObjectArray
{
    long pObjectVec;    // pointer to native C++ SSObjectVec

    public JSSObjectArray()
    {
        create();
    }

    public void finalize()
    {
        destroy();
    }

    public native void create();
    public native void destroy();
    public native int size();
    public native int importFromCSV ( String path );
    public native int exportToCSV ( String path );
}