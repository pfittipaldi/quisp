//Crude model for satellite visibility: this module checks whether the satellite should be visible or not at this time and sets the Line Of Sight
//parameter of Free-Space channels accordingly.

package modules.Satellite;
@namespace(quisp::modules::Satellite);
simple PointingSystem
{
parameters:
    string name = "PointingSystem";
    @class(PointingSystem);
gates:
    input req;
    output ans;
}
