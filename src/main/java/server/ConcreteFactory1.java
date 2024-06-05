package server;

public class ConcreteFactory1 implements AbstractFactory {

    @Override
    public Turbo1 createTurbo() {
        return new Turbo1();
    }

    @Override
    public Vida createVida() {
        // TODO Auto-generated method stub
        throw new UnsupportedOperationException("Unimplemented method 'createVida'");
    }

    @Override
    public Hueco createHueco() {
        // TODO Auto-generated method stub
        throw new UnsupportedOperationException("Unimplemented method 'createHueco'");
    }

    @Override
    public Bala createBala() {
        // TODO Auto-generated method stub
        throw new UnsupportedOperationException("Unimplemented method 'createBala'");
    }
}