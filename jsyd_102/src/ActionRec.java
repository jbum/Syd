// Used by HammerBank
public class ActionRec
{
  ModList instr;
  double energy; // String Energy - decays from 1.0 to 0
  double decay;  // Decay for energy
  double freq; // frequency for this key
  double amp;  // amplitude scale for this key
  double undampen; // typically 1 when undamped, 0 when damped
  double waveInc;
  double attack;
  double velocity;
  double attackEnergy;
  double decayEnergy;
  int   attackCtr,
        attackSamples,
        decayCtr,
        decaySamples;
  boolean attackFlag,
          decayFlag;
}
