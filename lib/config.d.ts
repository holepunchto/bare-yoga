/** Settings shared by the nodes created with it. */
interface YogaConfig extends Disposable {
  /** The tag that identifies the config to the binding. Zero once destroyed. */
  readonly tag: number

  /**
   * How many physical pixels there are to a point. Layout is rounded to whole
   * pixels, unless it is zero.
   */
  set pointScaleFactor(value: number)
  /** Whether to use the defaults of CSS rather than those of React Native. */
  set useWebDefaults(value: boolean)
  /** Which known bugs to keep, as a combination of `ERRATA` flags. */
  set errata(value: number)

  /** Free the config. Nodes created with it must be destroyed first. */
  destroy(): void
}

declare class YogaConfig {
  constructor(opts?: { pointScaleFactor?: number; useWebDefaults?: boolean; errata?: number })
}

export = YogaConfig
