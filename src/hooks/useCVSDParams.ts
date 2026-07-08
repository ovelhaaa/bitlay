import { useState, useCallback } from 'react';
import { DEFAULT_PARAMS, CVSDDelayParams } from '../types';
import { PRESETS } from '../lib/presets';
import { SUBDIVISIONS } from '../lib/constants';

export function useCVSDParams() {
  const [params, setParams] = useState<CVSDDelayParams>(DEFAULT_PARAMS);
  const [currentPreset, setCurrentPreset] = useState<string>('default');
  const [isBypassed, setIsBypassed] = useState(false);
  const [isCoupledMode, setIsCoupledMode] = useState(true);
  const [isFreeze, setIsFreeze] = useState(false);

  const loadPreset = useCallback((name: string) => {
    setCurrentPreset(name);
    const newParams = PRESETS[name] || DEFAULT_PARAMS;
    setParams(newParams);
  }, []);

  const handleParamChange = useCallback((key: keyof CVSDDelayParams, value: any) => {
    setCurrentPreset('custom');
    if (key === 'delayTime') {
      setParams((prev) => ({
        ...prev,
        delayTime: value,
        mainSubdivision: 'custom',
        bpmSync: false,
      }));
    } else {
      setParams((prev) => ({
        ...prev,
        [key]: value,
      }));
    }
  }, []);

  const handleNestedParamChange = useCallback(
    (mode: 'discrete' | 'companded', key: string, value: number) => {
      setCurrentPreset('custom');
      setParams((prev) => ({
        ...prev,
        [mode]: {
          ...prev[mode],
          [key]: value,
        },
      }));
    },
    [],
  );

  const handleTapParamChange = useCallback(
    (tapId: 1 | 2 | 3 | 4, field: 'multiplier' | 'mix', value: number) => {
      setCurrentPreset('custom');
      setParams((prev) => {
        const updatedTaps = { ...prev.taps };
        if (field === 'multiplier') {
          updatedTaps[tapId] = {
            ...prev.taps[tapId],
            multiplier: value,
            subdivision: 'custom',
          };
        } else {
          updatedTaps[tapId] = {
            ...prev.taps[tapId],
            mix: value,
          };
        }
        return {
          ...prev,
          taps: updatedTaps,
        };
      });
    },
    [],
  );

  const handleTapSubdivisionChange = useCallback(
    (tapId: 1 | 2 | 3 | 4, subdiv: keyof typeof SUBDIVISIONS) => {
      setCurrentPreset('custom');
      setParams((prev) => {
        const updatedTaps = { ...prev.taps };
        const currentTap = updatedTaps[tapId];

        let newMultiplier = currentTap.multiplier;
        if (subdiv !== 'custom') {
          const mainMult = SUBDIVISIONS[prev.mainSubdivision] || 1.0;
          const tapMult = SUBDIVISIONS[subdiv] || 1.0;
          newMultiplier = Math.max(0.1, Math.min(2.0, parseFloat((tapMult / mainMult).toFixed(2))));
        }

        updatedTaps[tapId] = {
          ...currentTap,
          subdivision: subdiv,
          multiplier: newMultiplier,
        };

        return {
          ...prev,
          taps: updatedTaps,
        };
      });
    },
    [],
  );

  const handleReset = useCallback(() => {
    loadPreset('default');
  }, [loadPreset]);

  return {
    params,
    setParams,
    currentPreset,
    isBypassed,
    setIsBypassed,
    isCoupledMode,
    setIsCoupledMode,
    isFreeze,
    setIsFreeze,
    loadPreset,
    handleParamChange,
    handleNestedParamChange,
    handleTapParamChange,
    handleTapSubdivisionChange,
    handleReset,
  };
}
