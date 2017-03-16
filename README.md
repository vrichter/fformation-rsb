# fformation-rsb

This project uses [fformation](https://github.com/vrichter/fformation) in
combination with rsb & rst to listen for `rst::hri::PersonHypotheses` and
publish assignments as `rst::hri::ConversationalGroupCollection`.

By checking out the [gco](https://github.com/vrichter/fformation-gco.git)
submodule the corresponding gco classificator can be used too (the license
  changes thereby).

## How do I get set up? ###

    > git clone --recursive
    > mkdir -p fformation-rsb/build && cd fformation-rsb/build
    > cmake .. && make

## Applications

### fformation-rsb-classificator

```bash
Allowed options:
  -h [ --help ]                      produce help message
  -c [ --classificator ] arg (=list) Which classificator should be used for
                                     evaluation. Possible:  ( gco | grow | none
                                     | one | shrink | )
  -i [ --inscope ] arg               The scope to listen for person hypotheses
  -o [ --outscope ] arg              The scope to publish group assignments
  -m [ --mdl ] arg (=40000)          The minimum description length prior.
  -s [ --stride ] arg (=10)          The distance btw. a persons position and
                                     trnsactional space.
```

This application listens for `rst::hri::PersonHypotheses` on `inscope`, assigns
the persons to groups and publishes a corresponding
`rst::hri::ConversationalGroupCollection` on `outscope`.

### 3rd party software used

* [Boost](http://www.boost.org/ "Boost C++ Libraries") because it is boost.
* [rsb](https://code.cor-lab.de/projects/rsb) IPC-Middleware
* [rst](https://code.cor-lab.de/projects/rst) Type specifications for rsb
* [rsb-experimental](https://projects.cit-ec.uni-bielefeld.de/projects/rst-experimental) Additional rst types in revision process for the real rst
* [gco-v3.0](https://github.com/vrichter/gco-v3.0) for the graph-cuts optimization
* [fformation](https://github.com/vrichter/fformation) for the evaluation part

## Copyright

When you are using the gco submodule please refer to [gco-v3.0](https://github.com/vrichter/gco-v3.0) if you want to
use this project.

Otherwise this project uses LGPLv3
