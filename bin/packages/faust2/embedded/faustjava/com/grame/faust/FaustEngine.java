/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.8
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.grame.faust;

public class FaustEngine implements FaustEngineConstants {
  public static String getLastError() {
    return FaustEngineJNI.getLastError();
  }

  public static dsp create1Dsp(String name_app, String dsp_content) {
    long cPtr = FaustEngineJNI.create1Dsp(name_app, dsp_content);
    return (cPtr == 0) ? null : new dsp(cPtr, false);
  }

  public static dsp create2Dsp(String name_app, String dsp_content, String argv, String target, int opt_level) {
    long cPtr = FaustEngineJNI.create2Dsp(name_app, dsp_content, argv, target, opt_level);
    return (cPtr == 0) ? null : new dsp(cPtr, false);
  }

  public static dsp create3Dsp() {
    long cPtr = FaustEngineJNI.create3Dsp();
    return (cPtr == 0) ? null : new dsp(cPtr, false);
  }

  public static void destroyDsp(dsp dsp) {
    FaustEngineJNI.destroyDsp(dsp.getCPtr(dsp), dsp);
  }

  public static boolean init1Dsp(dsp dsp, String name) {
    return FaustEngineJNI.init1Dsp(dsp.getCPtr(dsp), dsp, name);
  }

  public static boolean init2Dsp(dsp dsp, String name, int renderer, int sr, int bsize) {
    return FaustEngineJNI.init2Dsp(dsp.getCPtr(dsp), dsp, name, renderer, sr, bsize);
  }

  public static boolean startDsp(dsp dsp) {
    return FaustEngineJNI.startDsp(dsp.getCPtr(dsp), dsp);
  }

  public static void stopDsp(dsp dsp) {
    FaustEngineJNI.stopDsp(dsp.getCPtr(dsp), dsp);
  }

  public static int getNumInputsDsp(dsp dsp) {
    return FaustEngineJNI.getNumInputsDsp(dsp.getCPtr(dsp), dsp);
  }

  public static int getNumOutputsDsp(dsp dsp) {
    return FaustEngineJNI.getNumOutputsDsp(dsp.getCPtr(dsp), dsp);
  }

  public static void connectDsp(dsp dsp1, dsp dsp2, int src, int dst) {
    FaustEngineJNI.connectDsp(dsp.getCPtr(dsp1), dsp1, dsp.getCPtr(dsp2), dsp2, src, dst);
  }

  public static void disconnectDsp(dsp dsp1, dsp dsp2, int src, int dst) {
    FaustEngineJNI.disconnectDsp(dsp.getCPtr(dsp1), dsp1, dsp.getCPtr(dsp2), dsp2, src, dst);
  }

  public static boolean isConnectedDsp(dsp dsp1, dsp dsp2, int src, int dst) {
    return FaustEngineJNI.isConnectedDsp(dsp.getCPtr(dsp1), dsp1, dsp.getCPtr(dsp2), dsp2, src, dst);
  }

  public static String getJSONDsp(dsp dsp) {
    return FaustEngineJNI.getJSONDsp(dsp.getCPtr(dsp), dsp);
  }

  public static int getParamsCountDsp(dsp dsp) {
    return FaustEngineJNI.getParamsCountDsp(dsp.getCPtr(dsp), dsp);
  }

  public static int getParamIndexDsp(dsp dsp, String name) {
    return FaustEngineJNI.getParamIndexDsp(dsp.getCPtr(dsp), dsp, name);
  }

  public static String getParamAddressDsp(dsp dsp, int p) {
    return FaustEngineJNI.getParamAddressDsp(dsp.getCPtr(dsp), dsp, p);
  }

  public static String getParamUnitDsp(dsp dsp, int p) {
    return FaustEngineJNI.getParamUnitDsp(dsp.getCPtr(dsp), dsp, p);
  }

  public static float getParamMinDsp(dsp dsp, int p) {
    return FaustEngineJNI.getParamMinDsp(dsp.getCPtr(dsp), dsp, p);
  }

  public static float getParamMaxDsp(dsp dsp, int p) {
    return FaustEngineJNI.getParamMaxDsp(dsp.getCPtr(dsp), dsp, p);
  }

  public static float getParamStepDsp(dsp dsp, int p) {
    return FaustEngineJNI.getParamStepDsp(dsp.getCPtr(dsp), dsp, p);
  }

  public static float getParamValueDsp(dsp dsp, int p) {
    return FaustEngineJNI.getParamValueDsp(dsp.getCPtr(dsp), dsp, p);
  }

  public static void setParamValueDsp(dsp dsp, int p, float v) {
    FaustEngineJNI.setParamValueDsp(dsp.getCPtr(dsp), dsp, p, v);
  }

  public static float getParamRatioDsp(dsp dsp, int p) {
    return FaustEngineJNI.getParamRatioDsp(dsp.getCPtr(dsp), dsp, p);
  }

  public static void setParamRatioDsp(dsp dsp, int p, float v) {
    FaustEngineJNI.setParamRatioDsp(dsp.getCPtr(dsp), dsp, p, v);
  }

  public static void propagateAccDsp(dsp dsp, int acc, float a) {
    FaustEngineJNI.propagateAccDsp(dsp.getCPtr(dsp), dsp, acc, a);
  }

  public static void setAccConverterDsp(dsp dsp, int p, int acc, int curve, float amin, float amid, float amax) {
    FaustEngineJNI.setAccConverterDsp(dsp.getCPtr(dsp), dsp, p, acc, curve, amin, amid, amax);
  }

  public static void getAccConverterDsp(dsp dsp, int p, SWIGTYPE_p_int acc, SWIGTYPE_p_int curve, SWIGTYPE_p_float amin, SWIGTYPE_p_float amid, SWIGTYPE_p_float amax) {
    FaustEngineJNI.getAccConverterDsp(dsp.getCPtr(dsp), dsp, p, SWIGTYPE_p_int.getCPtr(acc), SWIGTYPE_p_int.getCPtr(curve), SWIGTYPE_p_float.getCPtr(amin), SWIGTYPE_p_float.getCPtr(amid), SWIGTYPE_p_float.getCPtr(amax));
  }

  public static void propagateGyrDsp(dsp dsp, int acc, float a) {
    FaustEngineJNI.propagateGyrDsp(dsp.getCPtr(dsp), dsp, acc, a);
  }

  public static void setGyrConverterDsp(dsp dsp, int p, int acc, int curve, float amin, float amid, float amax) {
    FaustEngineJNI.setGyrConverterDsp(dsp.getCPtr(dsp), dsp, p, acc, curve, amin, amid, amax);
  }

  public static void getGyrConverterDsp(dsp dsp, int p, SWIGTYPE_p_int acc, SWIGTYPE_p_int curve, SWIGTYPE_p_float amin, SWIGTYPE_p_float amid, SWIGTYPE_p_float amax) {
    FaustEngineJNI.getGyrConverterDsp(dsp.getCPtr(dsp), dsp, p, SWIGTYPE_p_int.getCPtr(acc), SWIGTYPE_p_int.getCPtr(curve), SWIGTYPE_p_float.getCPtr(amin), SWIGTYPE_p_float.getCPtr(amid), SWIGTYPE_p_float.getCPtr(amax));
  }

}
