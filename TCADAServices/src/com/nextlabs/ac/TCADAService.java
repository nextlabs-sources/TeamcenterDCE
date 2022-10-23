package com.nextlabs.ac;

import com.bluejungle.framework.expressions.*;
import com.bluejungle.pf.domain.destiny.serviceprovider.IFunctionServiceProvider;
import com.bluejungle.pf.domain.destiny.serviceprovider.ServiceProviderException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.json.JSONArray;
import org.json.JSONObject;

import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Iterator;

public class TCADAService implements IFunctionServiceProvider {

    private static final Log LOG = LogFactory.getLog(TCADAService.class);

    @Override
    public IEvalValue callFunction(String functionName, IEvalValue[] arguments) throws ServiceProviderException {
        IEvalValue result = EvalValue.EMPTY;

        try {

            LOG.info("TCADAService callFunction() started, with function: " + functionName);

            long lCurrentTime = System.nanoTime();

            if ("isExpired".equalsIgnoreCase(functionName)) {
                result = isExpired(arguments);
            } else if ("getLicenseByID".equalsIgnoreCase(functionName)) {
                ArrayList<String> inputList = processArgument(arguments);
                result = getLicenseByID(inputList);
            }

            LOG.info("TCADAService callFunction() completed. Result: "
                    + result.toString() + " Time spent: "
                    + ((System.nanoTime() - lCurrentTime) / 1000000.00) + "ms");
        } catch (Exception e) {
            LOG.error("TCADAService callFunction() error: ", e);
        }

        return result;
    }

    @Override
    public void init() {
        LOG.info("TCADAService init() started.");

        LOG.info("TCADAService init() completed.");
    }

    private IEvalValue getLicenseByID(ArrayList<String> argumentList) {
        LOG.info("TCADAService getLicenseID() started.");

        IEvalValue result = IEvalValue.EMPTY;

        String tcADALicense = "";
        String licenseID = "";

        if (argumentList.size() != 2) {
            LOG.info("TCADAService getLicenseID() error: Invalid number of argument number: " + argumentList.size());
        } else {
            if (argumentList.get(0) != null) {
                tcADALicense = argumentList.get(0);
                if (!tcADALicense.startsWith("[")) {
                    tcADALicense = "[" + tcADALicense;
                }
                if (!tcADALicense.endsWith("]")) {
                    tcADALicense = tcADALicense + "]";
                }
                LOG.info(tcADALicense);
            }
            if (argumentList.get(1) != null) {
                licenseID = argumentList.get(1);
                LOG.info(licenseID);
            }

            JSONArray licenseListJSON = new JSONArray(tcADALicense);

            for (int i = 0; i < licenseListJSON.length(); i++) {
                JSONObject licenseObject = licenseListJSON.getJSONObject(i);
                if (licenseObject.get("id").equals(licenseID)) {
                    result = EvalValue.build(licenseObject.toString());
                    break;
                }
            }
        }

        LOG.info("TCADAService getLicenseID() completed.");

        return result;
    }

    private IEvalValue isExpired(IEvalValue[] args) {
        LOG.info("TCADAService isExpired() started.");

        IEvalValue result = EvalValue.build("false");   // By default

        if (args[0] != null) {
            if (args[0] == IEvalValue.EMPTY) {
                // In case there was no particular license attached to the user, the whole condition should deny
                result = EvalValue.build("true");
            } else if (args[0].getType() == ValueType.STRING) {
                String license = args[0].getValue().toString();
                LOG.info(license);
                if (!license.contains("expiry_date")) {
                    LOG.info("There is no expiry date for this license");
                    result = EvalValue.build("false");
                } else {
                    JSONObject licenseObject = new JSONObject(license);
                    String expiryDate = licenseObject.get("expiry_date").toString();
                    LOG.info("License Expired at: " + expiryDate);
                    DateFormat dateFormat = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");

                    try {
                        Date expiredAt = dateFormat.parse(expiryDate);
                        Date now = new Date();
                        if (expiredAt.after(now)) {
                            result = EvalValue.build("false");
                        } else {
                            result = EvalValue.build("true");
                        }
                    } catch (ParseException ex) {
                        LOG.error("TCADAService isExpired() error: Unable to parse given expiry_date with proper format");
                    }
                }
            } else {
                // In case tc_ada_license can't be query
                result = EvalValue.build("true");
                LOG.debug("TCADAService isExpired() error: Argument license property is not a valid string");
            }
        }


        LOG.info("isExpired: " + result.getValue().toString());
        LOG.info("TCADAService isExpired() completed.");

        return result;
    }

    private ArrayList<String> processArgument(IEvalValue[] args) {
        LOG.info("TCADAService processArgument entered ");

        ArrayList<String> sOutData = new ArrayList<>();

        for (IEvalValue ieValue : args) {
            LOG.info("ieValue " + ieValue.toString());

            String sData = "";
            if (ieValue.getType() == ValueType.MULTIVAL) {
                ArrayList<String> tempList = new ArrayList<>();
                IMultivalue mv = (IMultivalue) ieValue.getValue();
                if (Multivalue.EMPTY == mv) {
                    sOutData.add("-9999");
                    continue;
                }

                for (Iterator<IEvalValue> it = mv.iterator(); it.hasNext();) {
                    IEvalValue ev = it.next();

                    if (ev.getType() == ValueType.STRING) {
                        LOG.info("Multival: " + ev.getValue().toString());
                        if (!ev.getValue().toString().isEmpty()) {
                            sData = ev.getValue().toString();
                        }

                    } else if (ev.getType() == ValueType.LONG) {
                        sData = ev.getValue().toString();
                    } else if (ev.getType() == ValueType.DATE) {
                        sData = ev.getValue().toString();
                    }
                    tempList.add(sData);
                }

                sOutData.add(join(tempList, ",", "", ""));
            } else if (ieValue.getType() == ValueType.STRING && !ieValue.getValue().toString().isEmpty()) {
                sData = ieValue.getValue().toString();

                sOutData.add(sData);
            }

        }
        LOG.info("Input Data: " + sOutData);
        return sOutData;
    }

    public static String join(ArrayList<String> list, String separator,
                              String prefix, String suffix) {
        StringBuilder sb = new StringBuilder();
        if (prefix != null && !prefix.isEmpty())
            sb.append(prefix);

        if (list.size() <= 0) {
            sb.append("");
        } else {
            int sz = list.size();
            for (int i = 0; i < sz; i++) {
                String str = list.get(i);
                if (i == (sz - 1)) {
                    if (str != null && !str.isEmpty())
                        sb.append(str);

                } else {
                    if (str != null && !str.isEmpty())
                        sb.append(str).append(separator);
                }
            }
        }

        if (suffix != null && !suffix.isEmpty())
            sb.append(suffix);

        return sb.toString();
    }


    public static void main(String[] args) throws Exception {
        TCADAService plugin = new TCADAService();
        plugin.init();

        IEvalValue[] licenseArgs = new IEvalValue[2];
        licenseArgs[0] = EvalValue.build("[{\"fnd0user_citizenships\":\"[AU, US]\",\"fnd0license_category\":\"Category C\",\"id\":\"ENG2\"}," +
                " {\"fnd0user_citizenships\":\"[AU, CN, US, VN]\",\"fnd0license_category\":\"Category C\",\"id\":\"ENG4\"}," +
                " {\"fnd0user_citizenships\":\"[AU, US, VN]\",\"expiry_date\":\"2021/03/15 00:00:00\",\"lock_date\":\"2022/11/15 00:00:00\",\"fnd0license_category\":\"Category B\",\"id\":\"ENG3\"}," +
                " {\"fnd0user_citizenships\":\"[US]\",\"fnd0license_category\":\"Category A\",\"id\":\"ENG1\"}]");
        licenseArgs[1] = EvalValue.build("ENG5");

        IEvalValue license = plugin.callFunction("getLicenseByID", licenseArgs);

        System.out.println(license);

        IEvalValue isExpired = plugin.callFunction("isExpired", new IEvalValue[] { license });

        System.out.println(isExpired);
    }
}
