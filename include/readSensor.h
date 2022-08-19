void readSensor()
{
#if SENSOR_1
   Distance = Sonar.read();
   mySensor.add(Distance);
   DistanceX = mySensor.get();
   if (Distance <= MaxDistance && Distance >= MinDistance)
   {
      uint8_t valueX = map(DistanceX, MinDistance, MaxDistance, 100, 0);
      value = valueX;
      errorCount = 0;
   }
   else if (DistanceX >= MaxDistance)
   {
      value = 0;
   }
   else if (Distance <= MinDistance)
   {
      value = 100;
   }
   if (Distance == 0 || Distance >= MaxDistance + 20) // if Error
   {
#if SW_TEST
      value = 80;
#else
      errorCount++;
      if (errorCount > 30)
      {
         errorCountState = true;
         errorCount = 1;
      }
#endif
   }
#elif SENSOR_2
   do
   {
      for (int i = 0; i < 4; i++)
      {
         data[i] = sensorSerial.read();
      }
   } while (sensorSerial.read() == 0xff);
   sensorSerial.flush();
   if (data[0] == 0xff)
   {
      int sum;
      sum = (data[0] + data[1] + data[2]) & 0x00FF;
      if (sum == data[3])
      {
         Distance = (data[1] << 8) + data[2];
         if (Distance > 30)
         {
            Distance /= 10;
            mySensor.add(Distance);
            DistanceX = mySensor.get();
            // if (Distance <= MaxDistance && Distance >= MinDistance)  //commented out to test >100%
            // {
            value = map(DistanceX, MinDistance, MaxDistance, 100, 0);
            // value = valueX;
            // }
            // else if (DistanceX >= MaxDistance)
            // {
            //    value = 0;
            // }
            // else if (DistanceX <= MinDistance)
            // {
            //    // value = 100;
            // }
            errorCount = 0;
            if (DryRunState == false)
               errorCountState = false;
         }
         else
         {
            debugln("Below the lower limit");
            errorCount++;
            if (errorCount > 20)
            {
               errorCountState = true;
               errorCount = 1;
            }
         }
      }
      else // if errors
      {
#if SW_TEST
         value = 40;
#else
         errorCount++;
         if (errorCount > 20)
         {
            errorCountState = true;
            errorCount = 1;
         }
#endif
      }
   }
#elif SENSOR_3
   digitalWrite(TRIGGER_PIN, LOW);
   delayMicroseconds(5);
   digitalWrite(TRIGGER_PIN, HIGH);
   delayMicroseconds(100);
   digitalWrite(TRIGGER_PIN, LOW);
   duration = pulseIn(PWM_OUTPUT_PIN, HIGH);

   Distance = duration;
   Distance = Distance / 58;
   mySensor.add(Distance);
   DistanceX = mySensor.get();
   if (Distance <= MaxDistance && Distance >= MinDistance)
   {
      uint8_t valueX = map(DistanceX, MinDistance, MaxDistance, 100, 0);
      value = valueX;
      errorCount = 0;
   }
   else if (DistanceX >= MaxDistance)
   {
      value = 0;
   }
   else if (DistanceX <= MinDistance)
   {
      value = 100;
   }
   if (Distance == 0 || Distance > 500) // if Error
   {
#if SW_TEST
      value = 40;
#else
      errorCount++;
      if (errorCount > 15)
      {
         errorCountState = true;
         errorCount = 1;
      }
#endif
   }
#endif
   if (value < 0)
   {
      value = 0;
   }
   else if (value > 100)
   {
      // value = 100;  //display more than 100%
   }
   if (value <= MotorStartThreshold && ManualOff == false && AutoMode == true && Distance != 0)
   {
      PumpON_command();
   }
   else if (DistanceX < MinDistance && Distance != 0)
   {
      PumpOFF_command();
   }
}