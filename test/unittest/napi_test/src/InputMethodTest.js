/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import inputMethod from '@ohos.inputMethod'
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe("InputMethodTest", function () {
    beforeAll(function () {
      console.info('beforeAll called')
      inputMethod.getSetting().on('imeChange', imeChange);
    })

    afterAll(function () {
      inputMethod.getSetting().off('imeChange')
      console.info('afterAll called')
    })

    beforeEach(function () {
      console.info('beforeEach called')
    })

    afterEach(function () {
      console.info('afterEach called')
    })
    let bundleName = "com.example.newTestIme";
    let extName = "InputMethodExtAbility";
    let subName = ["lowerInput", "upperInput", "chineseInput"];
    let locale = ["en-US", "en-US", "zh-CN"];
    let language = ["english", "english", "chinese"];
    let bundleName1 = "com.example.testIme";
    let extName1 = ["InputMethodExtAbility", "InputMethodExtAbility2"];
    let language1 = ["chinese", "english"];
    const LEAST_ALL_IME_NUM = 2;
    const ENABLE_IME_NUM = 1;
    const LEAST_DISABLE_IME_NUM = 1;
    const NEW_IME_SUBTYPE_NUM = 3;
    const OLD_IME_SUBTYPE_NUM = 2;
    const WAIT_DEAL_OK = 500;

    let isImeChange = false;
    let imeChangeProp = undefined;
    let imeChangeSubProp = undefined;
    function imeChange(prop, subProp) {
      isImeChange = true;
      imeChangeProp = prop;
      imeChangeSubProp = subProp;
    }

    function checkNewImeCurrentProp(property)
    {
      expect(property.name).assertEqual(bundleName);
      expect(property.id).assertEqual(extName);
      expect(property.packageName).assertEqual(bundleName);
      expect(property.methodId).assertEqual(extName);
    }

    function checkNewImeCurrentSubProp(subProp, index)
    {
      expect(subProp.name).assertEqual(bundleName);
      expect(subProp.id).assertEqual(subName[index]);
      expect(subProp.locale).assertEqual(locale[index]);
      expect(subProp.language).assertEqual(language[index]);
    }

    function checkNewImeSubProps(subProps)
    {
      expect(subProps.length).assertEqual(NEW_IME_SUBTYPE_NUM);
      for (let i = 0; i < subProps.length; i++) {
        expect(subProps[i].name).assertEqual(bundleName);
        expect(subProps[i].id).assertEqual(subName[i]);
        expect(subProps[i].locale).assertEqual(locale[i]);
        expect(subProps[i].language).assertEqual(language[i]);
      }
    }

    function checkImeCurrentProp(property, index)
    {
      expect(property.name).assertEqual(bundleName1);
      expect(property.id).assertEqual(extName1[index]);
      expect(property.packageName).assertEqual(bundleName1);
      expect(property.methodId).assertEqual(extName1[index]);
    }

    function checkImeCurrentSubProp(subProp, index)
    {
      expect(subProp.name).assertEqual(bundleName1);
      expect(subProp.id).assertEqual(extName1[index]);
      expect(subProp.locale).assertEqual("");
      expect(subProp.language).assertEqual(language1[index]);
    }

    function checkImeSubProps(subProps)
    {
      expect(subProps.length).assertEqual(OLD_IME_SUBTYPE_NUM);
      for (let i = 0; i < subProps.length; i++) {
        expect(subProps[i].name).assertEqual(bundleName1);
        expect(subProps[i].id).assertEqual(extName1[i]);
        expect(subProps[i].locale).assertEqual("");
        expect(subProps[i].language).assertEqual(language1[i]);
      }
    }

    function wait(delay) {
      let start = new Date().getTime();
      while (new Date().getTime() - start < delay){
      }
    }

    /*
     * @tc.number  inputmethod_test_MAX_TYPE_NUM_001
     * @tc.name    Test MAX_TYPE_NUM.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_MAX_TYPE_NUM_001', 0, async function (done) {
      console.info("************* inputmethod_test_MAX_TYPE_NUM_001 Test start*************");
      let MAX_NUM = 128;
      let num = inputMethod.MAX_TYPE_NUM;
      console.info("inputmethod_test_001 result:" + num);
      expect(num).assertEqual(MAX_NUM);
      console.info("************* inputmethod_test_MAX_TYPE_NUM_001 Test end*************");
      done();
    });

    /*
     * @tc.number  inputmethod_test_getInputMethodController_001
     * @tc.name    Test to get an InputMethodController instance.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_getInputMethodController_001', 0, async function (done) {
      console.info("************* inputmethod_test_getInputMethodController_001 Test start*************");
      let controller = inputMethod.getInputMethodController();
      expect(controller !== undefined).assertTrue();
      console.info("************* inputmethod_test_getInputMethodController_001 Test end*************");
      done();
    });

    /*
     * @tc.number  inputmethod_test_getController_001
     * @tc.name    Test to get an InputMethodController instance.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_getController_001', 0, async function (done) {
      console.info("************* inputmethod_test_getController_001 Test start*************");
      let controller = inputMethod.getController();
      expect(controller !== undefined).assertTrue();
      console.info("************* inputmethod_test_getController_001 Test end*************");
      done();
    });

    /*
     * @tc.number  inputmethod_test_getInputMethodSetting_001
     * @tc.name    Test to get an InputMethodSetting instance.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_getInputMethodSetting_001', 0, async function (done) {
      console.info("************* inputmethod_test_getInputMethodSetting_001 Test start*************");
      let setting = inputMethod.getInputMethodSetting();
      expect(setting !== undefined).assertTrue();
      console.info("************* inputmethod_test_getInputMethodSetting_001 Test end*************");
      done();
    });

    /*
     * @tc.number  inputmethod_test_getInputMethodSetting_001
     * @tc.name    Test to get an InputMethodSetting instance.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_getSetting_001', 0, async function (done) {
      console.info("************* inputmethod_test_getSetting_001 Test start*************");
      let setting = inputMethod.getSetting();
      expect(setting !== undefined).assertTrue();
      console.info("************* inputmethod_test_getSetting_001 Test end*************");
      done();
    });

    /*
     * @tc.number  inputmethod_test_switchInputMethod_001
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_switchInputMethod_001', 0, async function (done) {
      console.info("************* inputmethod_test_switchInputMethod_001 Test start*************");
      let inputMethodProperty = {
        name:bundleName,
        id:extName
      }
      inputMethod.switchInputMethod(inputMethodProperty).then(ret => {
        expect(ret).assertTrue();
        let property = inputMethod.getCurrentInputMethod();
        checkNewImeCurrentProp(property);
        console.info("************* inputmethod_test_switchInputMethod_001 Test end*************");
        wait(WAIT_DEAL_OK);
        done();
      }).catch( err=> {
        console.info("inputmethod_test_switchInputMethod_001 err:" + JSON.stringify(err.message));
        expect().assertFail();
      })
    });

    /*
    * @tc.number  inputmethod_test_listCurrentInputMethodSubtype_001
    * @tc.name    Test list current input method subtypes.
    * @tc.desc    Function test
    * @tc.level   2
    */
    it('inputmethod_test_listCurrentInputMethodSubtype_001', 0, async function (done) {
      console.info("************* inputmethod_test_listCurrentInputMethodSubtype_001 Test start*************");
      let inputMethodSetting = inputMethod.getSetting();
      inputMethodSetting.listCurrentInputMethodSubtype((err, subProps) => {
        if (err) {
          console.error("inputmethod_test_listCurrentInputMethodSubtype_001 err: " + JSON.stringify(err.message));
          return;
        }
        checkNewImeSubProps(subProps)
        console.info("************* inputmethod_test_listCurrentInputMethodSubtype_001 Test end*************");
        done();
      });
    });

    /*
     * @tc.number  inputmethod_test_listCurrentInputMethodSubtype_002
     * @tc.name    Test list current input method subtypes.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_listCurrentInputMethodSubtype_002', 0, async function (done) {
      console.info("************* inputmethod_test_listCurrentInputMethodSubtype_002 Test start*************");
      let inputMethodSetting = inputMethod.getSetting();
      inputMethodSetting.listCurrentInputMethodSubtype().then((subProps)=>{
        checkNewImeSubProps(subProps);
        console.info("************* inputmethod_test_listCurrentInputMethodSubtype_002 Test end*************");
        done();
      }).catch((err) => {
        console.info('inputmethod_test_listCurrentInputMethodSubtype_001 err ' + JSON.stringify(err.message));
        expect().assertFail();
      });
    });

    /*
     * @tc.number  inputmethod_test_listInputMethod_001
     * @tc.name    Test list input methods.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_listInputMethod_001', 0, async function (done) {
      console.info("************* inputmethod_test_listInputMethod_001 Test start*************");
      let inputMethodSetting = inputMethod.getInputMethodSetting();
      inputMethodSetting.listInputMethod((err, props) => {
        if (err) {
          console.error("inputmethod_test_listInputMethod_001 err: " + JSON.stringify(err.message));
          return;
        }
        expect(props.length >= LEAST_ALL_IME_NUM).assertTrue();
        let imeProp = props.filter(function (prop) {return prop.name === bundleName && prop.id === extName;});
        expect(imeProp.length).assertEqual(ENABLE_IME_NUM);
        let imeProp1 = props.filter(function (prop) {return prop.name === bundleName1;});
        expect(imeProp1.length).assertEqual(LEAST_DISABLE_IME_NUM);
        console.info("************* inputmethod_test_listInputMethod_001 Test end*************");
        done();
      });
    });

    /*
     * @tc.number  inputmethod_test_listInputMethod_002
     * @tc.name    Test list input methods.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_listInputMethod_002', 0, async function (done) {
      console.info("************* inputmethod_test_listInputMethod_002 Test start*************");
      let inputMethodSetting = inputMethod.getInputMethodSetting();
      await inputMethodSetting.listInputMethod().then((props) => {
        expect(props.length >= LEAST_ALL_IME_NUM).assertTrue();
        let imeProp = props.filter(function (prop) {return prop.name === bundleName && prop.id === extName;});
        expect(imeProp.length).assertEqual(ENABLE_IME_NUM);
        let imeProp1 = props.filter(function (prop) {return prop.name === bundleName1;});
        expect(imeProp1.length).assertEqual(LEAST_DISABLE_IME_NUM);
        console.info("************* inputmethod_test_listInputMethod_002 Test end*************");
        done();
      }).catch((err) => {
        console.info('inputmethod_test_listInputMethod_002 err ' + JSON.stringify(err.message));
        expect().assertFail();
      });
    });

    /*
     * @tc.number  inputmethod_test_getInputMethods_001
     * @tc.name    Test get enable input methods.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_getInputMethods_001', 0, async function (done) {
      console.info("************* inputmethod_test_getInputMethods_001 Test start*************");
      let inputMethodSetting = inputMethod.getInputMethodSetting();
      await inputMethodSetting.getInputMethods(true).then((props)=>{
        expect(props.length).assertEqual(ENABLE_IME_NUM);
        checkNewImeCurrentProp(props[0]);
        console.info("************* inputmethod_test_getInputMethods_001 Test end*************");
        done();
      }).catch((err) => {
        console.info('inputmethod_test_getInputMethods_001 err ' + JSON.stringify(err.message));
        expect().assertFail();
      });
    });

    /*
     * @tc.number  inputmethod_test_getInputMethods_002
     * @tc.name    Test get enable input methods.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_getInputMethods_002', 0, async function (done) {
      console.info("************* inputmethod_test_getInputMethods_002 Test start*************");
      let inputMethodSetting = inputMethod.getInputMethodSetting();
      inputMethodSetting.getInputMethods(true, (err, props) => {
        if (err) {
          console.error("inputmethod_test_getInputMethods_002 err: " + JSON.stringify(err.message));
          return;
        }
        expect(props.length).assertEqual(ENABLE_IME_NUM);
        checkNewImeCurrentProp(props[0]);
        console.info("************* inputmethod_test_getInputMethods_002 Test end*************");
        done();
      });
    });

    /*
     * @tc.number  inputmethod_test_getInputMethods_003
     * @tc.name    Test get disable input methods.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_getInputMethods_003', 0, async function (done) {
      console.info("************* inputmethod_test_getInputMethods_003 Test start*************");
      let inputMethodSetting = inputMethod.getInputMethodSetting();
      await inputMethodSetting.getInputMethods(false).then((props)=>{
        expect(props.length >= LEAST_DISABLE_IME_NUM).assertTrue();
        let imeProp = props.filter(function (prop) {return prop.name === bundleName1;});
        expect(imeProp.length).assertEqual(LEAST_DISABLE_IME_NUM);
        console.info("************* inputmethod_test_getInputMethods_003 Test end*************");
        done();
        }).catch((err) => {
          console.info('inputmethod_test_getInputMethods_003 err ' + JSON.stringify(err.message));
          expect().assertFail();
        });
      });

    /*
     * @tc.number  inputmethod_test_getInputMethods_004
     * @tc.name    Test get disable input methods.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_getInputMethods_004', 0, async function (done) {
      console.info("************* inputmethod_test_getInputMethods_004 Test start*************");
      let inputMethodSetting = inputMethod.getInputMethodSetting();
      inputMethodSetting.getInputMethods(false, (err, props) => {
        if (err) {
          console.error("inputmethod_test_getInputMethods_004 err: " + JSON.stringify(err.message));
          return;
        }
        expect(props.length >= LEAST_DISABLE_IME_NUM).assertTrue();
        let imeProp = props.filter(function (prop) {return prop.name === bundleName1;});
        expect(imeProp.length).assertEqual(LEAST_DISABLE_IME_NUM);
        console.info("************* inputmethod_test_getInputMethods_004 Test end*************");
        done();
      });
    });

    /*
     * @tc.number  inputmethod_test_switchCurrentInputMethodSubtype_001
     * @tc.name    Test Indicates the input method subtype which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_switchCurrentInputMethodSubtype_001', 0, async function (done) {
      console.info("************* inputmethod_test_switchCurrentInputMethodSubtype_001 Test start*************");
      let InputMethodSubtype = {
        name: bundleName,
        id: subName[1],
        locale:"en_US.ISO-8859-1",
        language:"en",
        extra:{},
      }
      inputMethod.switchCurrentInputMethodSubtype(InputMethodSubtype).then(ret => {
        expect(ret).assertTrue();
        let subProp = inputMethod.getCurrentInputMethodSubtype();
        checkNewImeCurrentSubProp(subProp, 1);
        console.info("************* inputmethod_test_switchCurrentInputMethodSubtype_001 Test end*************");
        wait(WAIT_DEAL_OK);
        done();
      }).catch( err=> {
        console.info("inputmethod_test_switchCurrentInputMethodSubtype_001 err:" + JSON.stringify(err.message))
        expect().assertFail();
      })
    });

    /*
     * @tc.number  inputmethod_test_switchCurrentInputMethodSubtype_002
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_switchCurrentInputMethodSubtype_002', 0, async function (done) {
      console.info("************* inputmethod_test_switchCurrentInputMethodSubtype_002 Test start*************");
      let InputMethodSubtype = {
        name:bundleName,
        id:subName[0],
        locale:"en_US.ISO-8859-1",
        language:"en",
        extra:{},
      }
      inputMethod.switchCurrentInputMethodSubtype(InputMethodSubtype, (err, ret)=>{
        if(err){
          console.info("inputmethod_test_switchCurrentInputMethodSubtype_002 error:" + JSON.stringify(err.message));
          return;
        }
        expect(ret).assertTrue();
        let subProp = inputMethod.getCurrentInputMethodSubtype();
        checkNewImeCurrentSubProp(subProp, 0);
        console.info("************* inputmethod_test_switchCurrentInputMethodSubtype_002 Test end*************");
        wait(WAIT_DEAL_OK);
        done();
      });
    });

    /*
     * @tc.number  inputmethod_test_imeChange_001
     * @tc.name    Test to subscribe 'imeChange'.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_imeChange_001', 0, async function (done) {
      console.info("************* inputmethod_test_imeChange_001 Test start*************");
      expect(isImeChange).assertTrue();
      let subProp = inputMethod.getCurrentInputMethodSubtype();
      let prop = inputMethod.getCurrentInputMethod();
      expect(imeChangeSubProp.name).assertEqual(subProp.name);
      expect(imeChangeSubProp.id).assertEqual(subProp.id);
      expect(imeChangeProp.name).assertEqual(prop.name);
      expect(imeChangeProp.id).assertEqual(prop.id);
      console.info("************* inputmethod_test_imeChange_001 Test end*************");
      done();
    });

    /*
     * @tc.number  inputmethod_test_switchCurrentInputMethodAndSubtype_001
     * @tc.name    Test Indicates the input method subtype which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_switchCurrentInputMethodAndSubtype_001', 0, async function (done) {
      console.info("************* inputmethod_test_switchCurrentInputMethodAndSubtype_001 Test start*************");
      let InputMethodSubtype = {
        name:bundleName1,
        id:extName1[0],
        locale:"en_US.ISO-8859-1",
        language:"en",
        extra:{},
      }
      let inputMethodProperty = {
        name:bundleName1,
        id:extName1[0],
      }
      inputMethod.switchCurrentInputMethodAndSubtype(inputMethodProperty, InputMethodSubtype).then(ret => {
        expect(ret).assertTrue();
        let subProp = inputMethod.getCurrentInputMethodSubtype();
        checkImeCurrentSubProp(subProp, 0);
        let property = inputMethod.getCurrentInputMethod();
        checkImeCurrentProp(property, 0)
        console.info("************* inputmethod_test_switchCurrentInputMethodAndSubtype_001 Test end*************");
        wait(WAIT_DEAL_OK);
        done();
      }).catch( err=> {
        console.info("inputmethod_test_switchCurrentInputMethodAndSubtype_001 err:" + JSON.stringify(err.message))
        expect().assertFail();
      })
    });

    /*
     * @tc.number  inputmethod_test_switchCurrentInputMethodAndSubtype_002
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_switchCurrentInputMethodAndSubtype_002', 0, async function (done) {
      console.info("************* inputmethod_test_switchCurrentInputMethodAndSubtype_002 Test start*************");
      let InputMethodSubtype = {
        name:bundleName,
        id:subName[2],
        locale:"en_US.ISO-8859-1",
        language:"en",
        extra:{},
      }
      let inputMethodProperty = {
        name:bundleName,
        id:extName
      }
      inputMethod.switchCurrentInputMethodAndSubtype(inputMethodProperty, InputMethodSubtype, (err, ret)=>{
        if(err){
          console.info("inputmethod_test_switchCurrentInputMethodAndSubtype_002 error:" + JSON.stringify(err.message));
          return;
        }
        expect(ret).assertTrue();
        let subProp = inputMethod.getCurrentInputMethodSubtype();
        checkNewImeCurrentSubProp(subProp, 2)
        let property = inputMethod.getCurrentInputMethod();
        checkNewImeCurrentProp(property);
        console.info("************* inputmethod_test_switchCurrentInputMethodAndSubtype_002 Test end*************");
        wait(WAIT_DEAL_OK);
        done();
      });
    });

    /*
    * @tc.number  inputmethod_test_listInputMethodSubtype_001
    * @tc.name    Test list input method subtypes.
    * @tc.desc    Function test
    * @tc.level   2
    */
    it('inputmethod_test_listInputMethodSubtype_001', 0, async function (done) {
      console.info("************* inputmethod_test_listInputMethodSubtype_001 Test start*************");
      let inputMethodProperty = {
        name:bundleName,
        id:extName
      }
      let inputMethodSetting = inputMethod.getSetting();
      inputMethodSetting.listInputMethodSubtype(inputMethodProperty, (err, subProps) => {
        if (err) {
          console.error("inputmethod_test_listInputMethodSubtype_001 err: " + JSON.stringify(err.message));
          return;
        }
        checkNewImeSubProps(subProps);
        console.info("************* inputmethod_test_listInputMethodSubtype_001 Test end*************");
        done();
      });
    });

    /*
     * @tc.number  inputmethod_test_listInputMethodSubtype_002
     * @tc.name    Test list input method subtypes.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_listInputMethodSubtype_002', 0, async function (done) {
      console.info("************* inputmethod_test_listInputMethodSubtype_002 Test start*************");
      let inputMethodProperty = {
        name:bundleName1,
        id:extName1[0]
      }
      let inputMethodSetting = inputMethod.getSetting();
      inputMethodSetting.listInputMethodSubtype(inputMethodProperty).then((subProps)=>{
        checkImeSubProps(subProps);
        console.info("************* inputmethod_test_listInputMethodSubtype_002 Test end*************");
        done();
      }).catch((err) => {
        console.info('inputmethod_test_listInputMethodSubtype_002 listInputMethodSubtype err ' + JSON.stringify(err.message));
        expect().assertFail();
      });
    });

    /*
     * @tc.number  inputmethod_test_attach_001
     * @tc.name    Test Indicates the input method which will show softboard with callback.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_attach_001', 0, async function (done) {
      console.info("************* inputmethod_test_attach_001 Test start*************");
      let inputMethodCtrl = inputMethod.getInputMethodController();
      let attribute = {
        textInputType: 0,
        enterKeyType: 1
      }
      let textConfig = {
        inputAttribute: attribute
      }
      inputMethodCtrl.attach(false, textConfig,(err)=>{
        if (err) {
          console.info('inputmethod_test_attach_001 err ' + JSON.stringify(err.message));
          return;
        }
        console.info("************* inputmethod_test_attach_001 Test end*************");
        done();
      });
    });

    /*
     * @tc.number  inputmethod_test_showSoftKeyboard_001
     * @tc.name    Test Indicates the input method which will show softboard with callback.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_showSoftKeyboard_001', 0, async function (done) {
      console.info("************* inputmethod_test_showSoftKeyboard_001 Test start*************");
      let inputMethodCtrl = inputMethod.getInputMethodController()
      inputMethodCtrl.showSoftKeyboard((err)=>{
        if (err) {
          console.info('inputmethod_test_showSoftKeyboard_001 err ' + JSON.stringify(err.message));
          return;
        }
        console.info("************* inputmethod_test_showSoftKeyboard_001 Test end*************");
        done();
      });
    });

    /*
     * @tc.number  inputmethod_test_hideSoftKeyboard_001
     * @tc.name    Test Indicates the input method which will hide softboard with callback.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_hideSoftKeyboard_001', 0, async function (done) {
      console.info("************* inputmethod_test_hideSoftKeyboard_001 Test start*************");
      let inputMethodCtrl = inputMethod.getInputMethodController()
      inputMethodCtrl.hideSoftKeyboard((err)=>{
        if(err){
          console.info('inputmethod_test_hideSoftKeyboard_001 err ' + JSON.stringify(err.message));
          return;
        }
        console.info("************* inputmethod_test_hideSoftKeyboard_001  Test end*************");
        done();
      });
    });

    /*
     * @tc.number  inputmethod_test_stopInputSession_001
     * @tc.name    Test Indicates the input method which will hides the keyboard.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_stopInputSession_001', 0, function (done) {
      console.info("************* inputmethod_test_stopInputSession_001 Test start*************");
      let inputMethodCtrl = inputMethod.getController();
      inputMethodCtrl.stopInputSession((err, ret) => {
        if (err) {
          console.info("inputmethod_test_stopInputSession_001 err" + JSON.stringify(err.message));
          return;
        }
        expect(ret).assertTrue();
        console.info("************* inputmethod_test_stopInputSession_001 Test end*************");
        done();
      });
    });

    /*
     * @tc.number  inputmethod_test_attach_002
     * @tc.name    Test Indicates the input method which will show softboard with callback.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_attach_002', 0, async function (done) {
      console.info("************* inputmethod_test_attach_002 Test start*************");
      let inputMethodCtrl = inputMethod.getInputMethodController();
      let attribute = {
        textInputType: 0,
        enterKeyType: 1
      }
      let textConfig = {
        inputAttribute: attribute
      }
      inputMethodCtrl.attach(false, textConfig).then(()=>{
        console.info("************* inputmethod_test_attach_002 Test end*************");
        done();
      }).catch((err) => {
        console.info("inputmethod_test_attach_002 err" + JSON.stringify(err.message));
        expect().assertFail();
      })
    });

    /*
     * @tc.number  inputmethod_test_showSoftKeyboard_002
     * @tc.name    Test Indicates the input method which will show softboard with Promise.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_showSoftKeyboard_002', 0, async function (done) {
      console.info("************* inputmethod_test_showSoftKeyboard_002 Test start*************");
      let inputMethodCtrl = inputMethod.getInputMethodController()
      inputMethodCtrl.showSoftKeyboard().then(() =>{
        console.info("************* inputmethod_test_showSoftKeyboard_002 Test end*************" );
        done();
      }).catch((err) => {
        console.info("inputmethod_test_showSoftKeyboard_002 err" + JSON.stringify(err.message));
        expect().assertFail();
      })
    });

    /*
     * @tc.number  inputmethod_test_hideSoftKeyboard_002
     * @tc.name    Test Indicates the input method which will hide softboard with Promise.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_hideSoftKeyboard_002', 0, async function (done) {
      console.info("************* inputmethod_test_hideSoftKeyboard_002 Test start*************");
      let inputMethodCtrl = inputMethod.getInputMethodController()
      inputMethodCtrl.hideSoftKeyboard().then(() =>{
        console.info("************* inputmethod_test_hideSoftKeyboard_002 Test end*************" );
        done();
      }).catch((err) => {
        console.info("inputmethod_test_hideSoftKeyboard_002 err" + JSON.stringify(err.message));
        expect().assertFail();
      })
    });

    /*
     * @tc.number  inputmethod_test_stopInputSession_002
     * @tc.name    Test Indicates the input method which will hides the keyboard.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_stopInputSession_002', 0, function (done) {
      console.info("************* inputmethod_test_stopInputSession_002 Test start*************");
      let inputMethodCtrl = inputMethod.getController();
      inputMethodCtrl.stopInputSession().then((ret) => {
        expect(ret).assertTrue();
        console.info("************* inputmethod_test_stopInputSession_002 Test end*************" );
        done();
      }).catch((err) => {
        console.info("inputmethod_test_stopInputSession_002 err" + JSON.stringify(err.message));
        expect().assertFail();
      })
    });

    /*
     * @tc.number  inputmethod_test_showOptionalInputMethods_001
     * @tc.name    Test displays a dialog box for selecting an input method.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_showOptionalInputMethods_001', 0, async function (done) {
      console.info("************* inputmethod_test_showOptionalInputMethods_001 Test start*************");
      let inputMethodSetting = inputMethod.getSetting();
      inputMethodSetting.showOptionalInputMethods((err) => {
        if(err){
          console.info("inputmethod_test_showOptionalInputMethods_001 err:" + JSON.stringify(err.message));
          return;
        }
        console.info("************* inputmethod_test_showOptionalInputMethods_001 Test end*************");
        done();
      });
    });

    /*
     * @tc.number  inputmethod_test_showOptionalInputMethods_002
     * @tc.name    Test displays a dialog box for selecting an input method.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_showOptionalInputMethods_002', 0, async function (done) {
      console.info("************* inputmethod_test_showOptionalInputMethods_002 Test start*************");
      let inputMethodSetting = inputMethod.getSetting();
      inputMethodSetting.showOptionalInputMethods().then(()=>{
        console.info("************* inputmethod_test_showOptionalInputMethods_002 Test end*************");
        wait(WAIT_DEAL_OK);
        done();
      }).catch((err) => {
        console.info('inputmethod_test_showOptionalInputMethods_002 err ' + JSON.stringify(err.message));
        expect().assertFail();
      });
    });
})