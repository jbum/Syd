public class HammerBankRec
{
  ActionRec[] keys = new ActionRec[SSHammerBank.MaxKeysPerBank];

  public HammerBankRec()
  {
    for (int i = 0; i < SSHammerBank.MaxKeysPerBank; ++i)
    {
      keys[i] = new ActionRec();
    }
  }
}