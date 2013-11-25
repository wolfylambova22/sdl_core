package com.ford.syncV4.proxy.rpc;

import android.test.InstrumentationTestCase;

import com.ford.syncV4.exception.SyncException;
import com.ford.syncV4.proxy.SyncProxyALM;
import com.ford.syncV4.proxy.interfaces.IProxyListenerALM;

import org.json.JSONException;
import org.json.JSONObject;

/**
 * Common methods for testing classes.
 *
 * Created by enikolsky on 2013-10-29.
 */
public class TestCommon {
    public static JSONObject paramsToRequestObject(JSONObject paramsObject)
            throws JSONException {
        JSONObject jsonObject = new JSONObject();
        JSONObject requestObject = new JSONObject();
        jsonObject.put("request", requestObject);
        requestObject.put("parameters", paramsObject);
        return jsonObject;
    }

    public static void setupMocking(InstrumentationTestCase testCase) {
        System.setProperty("dexmaker.dexcache", testCase.getInstrumentation()
                                                        .getTargetContext()
                                                        .getCacheDir()
                                                        .getPath());
    }

    public static SyncProxyALM getSyncProxyALMNoTransport(
            IProxyListenerALM proxyListener) throws SyncException {
        // we use custom subclass here to override the initializeProxy() method
        // to avoid using a transport
        return new SyncProxyALM(proxyListener, "!", null, null, true, null,
                null, null, null, null, false, null) {
            @Override
            protected void initializeProxy() throws SyncException {
            }
        };
    }
}