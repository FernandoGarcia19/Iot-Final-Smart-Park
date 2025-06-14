const Alexa = require('ask-sdk-core');
const AWS = require('aws-sdk');
const dynamoDb = new AWS.DynamoDB.DocumentClient();
const IotData = new AWS.IotData({ endpoint: 'agm6c372inpl5-ats.iot.us-east-2.amazonaws.com' });

async function getThingNames(userId) {
  const params = {
      TableName: 'userThings', 
      Key: { userId: userId }
  };

  try {
      const result = await dynamoDb.get(params).promise();
      if (!result.Item) return { entrance: null, exit: null };
      
      return {
          entrance: result.Item.entrance_thing_name || null,
          exit: result.Item.exit_thing_name || null
      };
  } catch (error) {
      console.error('Error obtaining ThingNames from DynamoDB: ', error);
      throw new Error('Couldnt obtain ThingNames from DynamoDB');
  }
};

async function getShadowParams(userId, gateType) {
  const thingNames = await getThingNames(userId);
  
  let thingName;
  if (gateType === 'entrance') {
    thingName = thingNames.entrance;
  } else if (gateType === 'exit') {
    thingName = thingNames.exit;
  } else {
    throw new Error('Invalid gate type specified. Use "entrance" or "exit"');
  }
  
  console.log(`Using ${gateType} gate: ${thingName}`);
  
  if (!thingName) {
      throw new Error(`${gateType} thing name no encontrado`);
  }
  
  return { thingName: thingName };
};

function getShadowPromise(params) {
  return new Promise((resolve, reject) => {
      IotData.getThingShadow(params, (err, data) => {
          if (err) {
              console.log(err, err.stack);
              reject('Error al obtener el shadow: ' + err.message);
          } else {
              resolve(JSON.parse(data.payload));
          }
      });
  });
};

function updateShadowPromise(params) {
  return new Promise((resolve, reject) => {
      IotData.updateThingShadow(params, (err, data) => {
          if (err) {
              console.log(err, err.stack);
              reject('Error al actualizar el shadow: ' + err.message);
          } else {
              resolve(data);
          }
      });
  });
};

const createGateHandler = (intentName, gateType, requestType) => {
    return {
        canHandle(handlerInput) {
            return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
                && Alexa.getIntentName(handlerInput.requestEnvelope) === intentName;
        },
        async handle(handlerInput) {
            const isConfig = requestType === 'config';
            let speakOutput = isConfig ? "Couldn't obtain the lapsus configuration" : "Couldn't obtain the gate state";
            
            try {
                const userId = handlerInput.requestEnvelope.session.user.userId;
                const params = await getShadowParams(userId, gateType);
                const shadowData = await getShadowPromise(params);
                
                if (isConfig) {
                    const lapsus = shadowData.state.reported.config.lapsus;
                    const threshold = shadowData.state.reported.config.threshold;
                    speakOutput = `The current sensor lapsus is ${lapsus} seconds and threshold is ${threshold}`;
                } else {
                    const state = shadowData.state.reported.gate_actuator;
                    const gateName = gateType.charAt(0).toUpperCase() + gateType.slice(1); // Capitalize first letter
                    speakOutput = `The ${gateName} gate is currently ${state}.`;
                }
            }
            catch(error) {
                console.error(`Error obtaining ${requestType} for ${gateType} gate`, error);
            }
            
            return handlerInput.responseBuilder
                .speak(speakOutput)
                .reprompt(speakOutput)
                .getResponse();
        }
    };
};

const getConfigEntranceHandler = createGateHandler('getConfigEntranceIntent', 'entrance', 'config');
const getConfigExitHandler = createGateHandler('getConfigExitIntent', 'exit', 'config');
const getEntranceGateStateHandler = createGateHandler('getEntranceGateStateIntent', 'entrance', 'state');
const getExitGateStateHandler = createGateHandler('getExitGateStateIntent', 'exit', 'state');

const createGateControlHandler = (intentName, gateType, action) => {
    return {
        canHandle(handlerInput) {
            return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
                && Alexa.getIntentName(handlerInput.requestEnvelope) === intentName;
        },
        async handle(handlerInput) {
            const userId = handlerInput.requestEnvelope.session.user.userId;
            const ShadowParams = await getShadowParams(userId, gateType);
            const payload = {
                state: {
                    desired: {
                        gate_actuator: action.toUpperCase()
                    }
                }
            };
            const params = {
                ...ShadowParams,
                payload: JSON.stringify(payload)
            };

            const gateName = gateType.charAt(0).toUpperCase() + gateType.slice(1);
            const pastTense = action === 'open' ? 'opened' : 'closed';

            try {
                await updateShadowPromise(params);
                const speakOutput = `The ${gateType} gate has been ${pastTense}.`;
                return handlerInput.responseBuilder
                    .speak(speakOutput)
                    .reprompt(speakOutput)
                    .getResponse();
            } catch (error) {
                console.error(`Error ${action}ing ${gateType}`, error);
                const speakOutput = `There was an error ${action}ing the ${gateType} gate. Please try again later.`;
                return handlerInput.responseBuilder
                    .speak(speakOutput)
                    .reprompt(speakOutput)
                    .getResponse();
            }
        }
    };
};

const createConfigSetterHandler = (intentName, gateType, configType) => {
    return {
        canHandle(handlerInput) {
            return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
                && Alexa.getIntentName(handlerInput.requestEnvelope) === intentName;
        },
        async handle(handlerInput) {
            const slotName = configType === 'threshold' ? 'threshold' : 'lapsus';
            const configValue = handlerInput.requestEnvelope.request.intent.slots[slotName].value;
            
            if (!configValue) {
                const speakOutput = `I couldn't understand the ${configType} value. Please try again.`;
                return handlerInput.responseBuilder
                    .speak(speakOutput)
                    .reprompt(speakOutput)
                    .getResponse();
            }
            
            const userId = handlerInput.requestEnvelope.session.user.userId;
            const ShadowParams = await getShadowParams(userId, gateType);
            const payload = {
                state: {
                    desired: {
                        config: {
                            [configType]: parseInt(configValue, 10)
                        }
                    }
                }
            };
            const params = {
                ...ShadowParams,
                payload: JSON.stringify(payload)
            };

            try {
                await updateShadowPromise(params);
                const speakOutput = `The ${configType} has been set to ${configValue}`;
                return handlerInput.responseBuilder
                    .speak(speakOutput)
                    .reprompt(speakOutput)
                    .getResponse();
            } catch (error) {
                console.error(`Error updating ${configType}`, error);
                const speakOutput = `There was an error updating the ${configType}. Please try again later.`;
                return handlerInput.responseBuilder
                    .speak(speakOutput)
                    .reprompt(speakOutput)
                    .getResponse();
            }
        }
    };
};

const openEntranceGateHandler = createGateControlHandler('openEntranceGateIntent', 'entrance', 'open');
const closeEntranceGateHandler = createGateControlHandler('closeEntranceGateIntent', 'entrance', 'closed');
const setEntranceThresholdHandler = createConfigSetterHandler('setEntranceThresholdIntent', 'entrance', 'threshold');
const setEntranceLapsusHandler = createConfigSetterHandler('setLapsusIntent', 'entrance', 'lapsus');

const openExitGateHandler = createGateControlHandler('openExitGateIntent', 'exit', 'open');
const closeExitGateHandler = createGateControlHandler('closeExitGateIntent', 'exit', 'closed');
const setExitThresholdHandler = createConfigSetterHandler('setExitThresholdIntent', 'exit', 'threshold');
const setExitLapsusHandler = createConfigSetterHandler('setExitLapsusIntent', 'exit', 'lapsus');

const LaunchRequestHandler = {
    canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'LaunchRequest';
    },
    handle(handlerInput) {
        const speakOutput = 'Welcome to your Smart Parking Assistant.';

        return handlerInput.responseBuilder
            .speak(speakOutput)
            .reprompt(speakOutput)
            .getResponse();
    }
};

const HelpIntentHandler = {
    canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
            && Alexa.getIntentName(handlerInput.requestEnvelope) === 'AMAZON.HelpIntent';
    },
    handle(handlerInput) {
        const speakOutput = 'You can say hello to me! How can I help?';

        return handlerInput.responseBuilder
            .speak(speakOutput)
            .reprompt(speakOutput)
            .getResponse();
    }
};

const CancelAndStopIntentHandler = {
    canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
            && (Alexa.getIntentName(handlerInput.requestEnvelope) === 'AMAZON.CancelIntent'
                || Alexa.getIntentName(handlerInput.requestEnvelope) === 'AMAZON.StopIntent');
    },
    handle(handlerInput) {
        const speakOutput = 'Goodbye!';

        return handlerInput.responseBuilder
            .speak(speakOutput)
            .getResponse();
    }
};


const FallbackIntentHandler = {
    canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
            && Alexa.getIntentName(handlerInput.requestEnvelope) === 'AMAZON.FallbackIntent';
    },
    handle(handlerInput) {
        const speakOutput = 'Sorry, I don\'t know about that. Please try again.';

        return handlerInput.responseBuilder
            .speak(speakOutput)
            .reprompt(speakOutput)
            .getResponse();
    }
};
/* *
 * SessionEndedRequest notifies that a session was ended. This handler will be triggered when a currently open 
 * session is closed for one of the following reasons: 1) The user says "exit" or "quit". 2) The user does not 
 * respond or says something that does not match an intent defined in your voice model. 3) An error occurs 
 * */
const SessionEndedRequestHandler = {
    canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'SessionEndedRequest';
    },
    handle(handlerInput) {
        console.log(`~~~~ Session ended: ${JSON.stringify(handlerInput.requestEnvelope)}`);
        // Any cleanup logic goes here.
        return handlerInput.responseBuilder.getResponse(); // notice we send an empty response
    }
};
/* *
 * The intent reflector is used for interaction model testing and debugging.
 * It will simply repeat the intent the user said. You can create custom handlers for your intents 
 * by defining them above, then also adding them to the request handler chain below 
 * */
const IntentReflectorHandler = {
    canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest';
    },
    handle(handlerInput) {
        const intentName = Alexa.getIntentName(handlerInput.requestEnvelope);
        const speakOutput = `You just triggered ${intentName}`;

        return handlerInput.responseBuilder
            .speak(speakOutput)
            //.reprompt('add a reprompt if you want to keep the session open for the user to respond')
            .getResponse();
    }
};
/**
 * Generic error handling to capture any syntax or routing errors. If you receive an error
 * stating the request handler chain is not found, you have not implemented a handler for
 * the intent being invoked or included it in the skill builder below 
 * */
const ErrorHandler = {
    canHandle() {
        return true;
    },
    handle(handlerInput, error) {
        const speakOutput = 'Sorry, I had trouble doing what you asked. Please try again.';
        console.log(`~~~~ Error handled: ${JSON.stringify(error)}`);

        return handlerInput.responseBuilder
            .speak(speakOutput)
            .reprompt(speakOutput)
            .getResponse();
    }
};

/**
 * This handler acts as the entry point for your skill, routing all request and response
 * payloads to the handlers above. Make sure any new handlers or interceptors you've
 * defined are included below. The order matters - they're processed top to bottom 
 * */
exports.handler = Alexa.SkillBuilders.custom()
    .addRequestHandlers(
        // Gate state handlers
        getEntranceGateStateHandler,
        getExitGateStateHandler,
        // Config handlers
        getConfigEntranceHandler,
        getConfigExitHandler,
        // Gate control handlers
        openEntranceGateHandler,
        closeEntranceGateHandler,
        openExitGateHandler,
        closeExitGateHandler,
        // Config setter handlers
        setEntranceThresholdHandler,
        setEntranceLapsusHandler,
        setExitThresholdHandler,
        setExitLapsusHandler,
        // Standard handlers
        LaunchRequestHandler,
        HelpIntentHandler,
        CancelAndStopIntentHandler,
        FallbackIntentHandler,
        SessionEndedRequestHandler,
        IntentReflectorHandler)
    .addErrorHandlers(
        ErrorHandler)
    .withCustomUserAgent('sample/hello-world/v1.2')
    .lambda();