import { useState, useEffect, useCallback } from 'react';
import { CVSDDelayParams } from '../types';
import { SUBDIVISIONS } from '../lib/constants';

export function useTapTempo(
  params: CVSDDelayParams,
  setParams: React.Dispatch<React.SetStateAction<CVSDDelayParams>>,
) {
  const [tapTimes, setTapTimes] = useState<number[]>([]);
  const [pulse, setPulse] = useState(false);

  useEffect(() => {
    const intervalTime = Math.max(50, params.delayTime);
    const interval = setInterval(() => {
      setPulse(true);
      const timeout = setTimeout(() => {
        setPulse(false);
      }, 95);
      return () => clearTimeout(timeout);
    }, intervalTime);

    return () => clearInterval(interval);
  }, [params.delayTime]);

  const updateBpmAndDelay = useCallback(
    (newBpm: number, isSynced: boolean, subdiv: keyof typeof SUBDIVISIONS) => {
      setParams((prev) => {
        const updated = { ...prev, bpm: newBpm, bpmSync: isSynced, mainSubdivision: subdiv };

        if (isSynced && subdiv !== 'custom') {
          const mult = SUBDIVISIONS[subdiv] || 1.0;
          const calculatedDelay = Math.round((60000 / newBpm) * mult);
          updated.delayTime = Math.max(10, Math.min(2000, calculatedDelay));

          const newTaps = { ...prev.taps };
          for (let t = 1; t <= 4; t++) {
            const tapId = t as 1 | 2 | 3 | 4;
            const tapSubdiv = prev.taps[tapId].subdivision;
            if (tapSubdiv && tapSubdiv !== 'custom') {
              const tapMult = SUBDIVISIONS[tapSubdiv] / mult;
              newTaps[tapId] = {
                ...prev.taps[tapId],
                multiplier: Math.max(0.1, Math.min(2.0, parseFloat(tapMult.toFixed(2)))),
              };
            }
          }
          updated.taps = newTaps;
        }

        return updated;
      });
    },
    [setParams],
  );

  const handleTapTempo = useCallback(() => {
    const now = performance.now();
    setTapTimes((prev) => {
      if (prev.length > 0 && now - prev[prev.length - 1] > 2500) {
        return [now];
      }
      const newTimes = [...prev, now];
      const sliced = newTimes.slice(-4);

      if (sliced.length >= 2) {
        const intervals: number[] = [];
        for (let i = 1; i < sliced.length; i++) {
          intervals.push(sliced[i] - sliced[i - 1]);
        }
        const avgInterval = intervals.reduce((sum, val) => sum + val, 0) / intervals.length;
        const calculatedBpm = Math.round(60000 / avgInterval);

        if (calculatedBpm >= 30 && calculatedBpm <= 300) {
          updateBpmAndDelay(calculatedBpm, true, params.mainSubdivision);
        }
      }
      return sliced;
    });
  }, [params.mainSubdivision, updateBpmAndDelay]);

  return { pulse, tapTimes, handleTapTempo, updateBpmAndDelay };
}
