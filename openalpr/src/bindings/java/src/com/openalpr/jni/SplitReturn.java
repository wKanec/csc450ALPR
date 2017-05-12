package com.openalpr.jni;

/**
 * Created by matt on 5/11/17.
 */
public class SplitReturn {
    public byte[] img;
    public SplitRect rect;
    public SplitReturn(byte[] img, SplitRect rect) {
        this.img = img;
        this.rect = rect;
    }
}
