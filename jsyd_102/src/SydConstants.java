public interface SydConstants
{
	static final int MT_Output 			= 0;
	static final int MT_Oscillator 		= 1;
	static final int MT_Envelope 		= 2;
	static final int MT_Mixer 			= 3;
	static final int MT_Filter 			= 4;
	static final int MT_Butter 			= 5;
	static final int MT_Smooth 			= 6;
	static final int MT_Noise 			= 7;
	static final int MT_Delay 			= 8;
	static final int MT_Threshhold 		= 9;
	static final int MT_SampleAndHold 	= 10;
	static final int MT_Amplifier 		= 11;
	static final int MT_Inverter 		= 12;
	static final int MT_Expression 		= 13;
	static final int MT_Folder 			= 14;
	static final int MT_RandScore 		= 15;
	static final int MT_CScore 			= 16;
	static final int MT_FInput 			= 17;
	static final int MT_PInput 			= 18;
	static final int MT_FTable 			= 19;
	static final int MT_SampleFile 		= 20;
	static final int MT_Pluck 			= 21;
	static final int MT_Maraca 			= 22;
	static final int MT_HammerBank 		= 23;
	static final int MT_HammerActuator 	= 24;
	static final int MT_GAssign 		= 25;
	static final int MT_SkiniScore 		= 26;
	static final int MT_STK 			= 27;
	static final int MT_NbrModules 		= 27; // Not including STK

  static final int WS_Idle        = 0;
  static final int WS_Synthesize  = 1;
  static final int WS_Playback    = 2;
  static final int WS_Abort       = 3;

  static final int MaxGlobals = 256;

}

