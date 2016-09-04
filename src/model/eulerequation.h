#ifndef _EULEREQUATION_H
#define _EULEREQUATION_H
#include "flux.h"
#include "reconstruction.h"
#include "bc.h"
#include "common.h"
#include "fluid.h"

template<class Tx, class Tad>
class EulerEquation{
public:
	uint ni, nj, nic, njc, nq;
	Array2D<Tad> rho, u, v, p, T, mu, k;
	Array3D<Tad> grad_u, grad_v, grad_T;
	Array2D<Tad> rholft_chi, ulft_chi, vlft_chi, plft_chi;
	Array2D<Tad> rhorht_chi, urht_chi, vrht_chi, prht_chi;
	Array2D<Tad> rholft_eta, ulft_eta, vlft_eta, plft_eta;
	Array2D<Tad> rhorht_eta, urht_eta, vrht_eta, prht_eta;
	Array3D<Tad> flux_chi, flux_eta;

	Array3D<Tad> grad_u_chi, grad_u_eta;
	Array3D<Tad> grad_v_chi, grad_v_eta;
	Array3D<Tad> grad_T_chi, grad_T_eta;
	Array2D<Tad> u_bar_chi, u_bar_eta;
	Array2D<Tad> v_bar_chi, v_bar_eta;
	Array2D<Tad> T_bar_chi, T_bar_eta;
	Array2D<Tad> mu_bar_chi, mu_bar_eta;
	Array2D<Tad> k_bar_chi, k_bar_eta;
	
	std::shared_ptr<Mesh<Tx>> mesh;
	std::shared_ptr<Config<Tx>> config;
	std::unique_ptr<Reconstruction<Tx, Tad>> reconstruction;
	std::unique_ptr<ConvectiveFlux<Tx, Tad>> convective_flux;
	std::unique_ptr<DiffusiveFlux<Tx, Tad>> diffusive_flux;
	std::shared_ptr<FluidModel<Tx, Tad>> fluid_model;
	std::unique_ptr<BoundaryContainer<Tx, Tad>> boundary_container;


	void calc_residual(Array3D<Tad>& a_q, Array3D<Tad>& a_rhs);
	void calc_convective_residual(Array3D<Tad>& a_rhs);
	void calc_intermediates(Array3D<Tad>& a_q);
	void calc_viscous_residual(Array3D<Tad>& a_rhs){
		diffusive_flux->evaluate(mesh->normal_chi, grad_u_chi, grad_v_chi, grad_T_chi, u_bar_chi, v_bar_chi, mu_bar_chi, k_bar_chi, flux_chi);
		diffusive_flux->evaluate(mesh->normal_eta, grad_u_eta, grad_v_eta, grad_T_eta, u_bar_eta, v_bar_eta, mu_bar_eta, k_bar_eta, flux_eta);

		for(uint i=0; i< nic; i++){
			for(uint j=0; j< njc; j++){
				for(uint k=1; k<mesh->solution->nq; k++){
					a_rhs[i][j][k] += (flux_eta[i][j+1][k] - flux_eta[i][j][k]);
					a_rhs[i][j][k] += (flux_chi[i+1][j][k] - flux_chi[i][j][k]);
				}
			}
		}

	};
	void calc_source_residual(Array3D<Tad>& a_q, Array3D<Tad>& a_rhs){
		for(uint i=0; i< nic; i++){
			for(uint j=0; j< njc; j++){
				a_rhs[i][j][1] += -config->solver->dpdx*mesh->volume[i][j];
				a_rhs[i][j][2] += -config->solver->dpdy*mesh->volume[i][j];
			}
		}
	};
	void calc_primvars(Array3D<Tad>& a_q);
	void calc_boundary();
	EulerEquation(std::shared_ptr<Mesh<Tx>> val_mesh, std::shared_ptr<Config<Tx>> val_config){
		mesh = val_mesh;
		config = val_config;

		ni = mesh->ni;
		nj = mesh->nj;
		nq = mesh->solution->nq;
		nic = ni - 1;
		njc = nj - 1;
		
		rho = Array2D<Tad>(nic+2, njc+2);
		u = Array2D<Tad>(nic+2, njc+2);
		v = Array2D<Tad>(nic+2, njc+2);
		p = Array2D<Tad>(nic+2, njc+2);
		T = Array2D<Tad>(nic+2, njc+2);
		mu = Array2D<Tad>(nic+2, njc+2);
		k = Array2D<Tad>(nic+2, njc+2);
		
		rholft_chi = Array2D<Tad>(ni, njc);
		ulft_chi = Array2D<Tad>(ni, njc);
		vlft_chi = Array2D<Tad>(ni, njc);
		plft_chi = Array2D<Tad>(ni, njc);
		
		rhorht_chi = Array2D<Tad>(ni, njc);
		urht_chi = Array2D<Tad>(ni, njc);
		vrht_chi = Array2D<Tad>(ni, njc);
		prht_chi = Array2D<Tad>(ni, njc);
		
		rholft_eta = Array2D<Tad>(nic, nj);
		ulft_eta = Array2D<Tad>(nic, nj);
		vlft_eta = Array2D<Tad>(nic, nj);
		plft_eta = Array2D<Tad>(nic, nj);
		
		rhorht_eta = Array2D<Tad>(nic, nj);
		urht_eta = Array2D<Tad>(nic, nj);
		vrht_eta = Array2D<Tad>(nic, nj);
		prht_eta = Array2D<Tad>(nic, nj);

		grad_u_chi = Array3D<Tad>(ni, njc, 2);
		grad_u_eta = Array3D<Tad>(nic, nj, 2);

		grad_v_chi = Array3D<Tad>(ni, njc, 2);
		grad_v_eta = Array3D<Tad>(nic, nj, 2);

		grad_T_chi = Array3D<Tad>(ni, njc, 2);
		grad_T_eta = Array3D<Tad>(nic, nj, 2);

		mu_bar_chi = Array2D<Tad>(ni, njc);
		k_bar_chi = Array2D<Tad>(ni, njc);
		u_bar_chi = Array2D<Tad>(ni, njc);
		v_bar_chi = Array2D<Tad>(ni, njc);
		T_bar_chi = Array2D<Tad>(ni, njc);

		mu_bar_eta = Array2D<Tad>(nic, nj);
		k_bar_eta = Array2D<Tad>(nic, nj);
		u_bar_eta = Array2D<Tad>(nic, nj);
		v_bar_eta = Array2D<Tad>(nic, nj);
		T_bar_eta = Array2D<Tad>(nic, nj);
			
		grad_u = Array3D<Tad>(nic, njc, 3);
		grad_v = Array3D<Tad>(nic, njc, 3);
		grad_T = Array3D<Tad>(nic, njc, 3);
		

		flux_chi = Array3D<Tad>(ni, njc, 4U);
		flux_eta = Array3D<Tad>(nic, nj, 4U);

		//transport = std::make_unique<TransportEquation<Tx, Tad>>(mesh, config);

		
		if(config->solver->order == 1){
			reconstruction = std::make_unique<FirstOrder<Tx, Tad>>(ni, nj);
		}
		else if(config->solver->order == 2){
			reconstruction = std::make_unique<SecondOrder<Tx, Tad>>(ni, nj);
		}
		else{
			
		}


		
		if(config->solver->flux == "roe")
			convective_flux = std::make_unique<RoeFlux<Tx, Tad>>();
		else if(config->solver->flux == "ausm")
			convective_flux = std::make_unique<AUSMFlux<Tx, Tad>>();
		else
			spdlog::get("console")->critical("Flux not found.");


		diffusive_flux = std::make_unique<GreeGaussFlux<Tx, Tad>>();
		fluid_model = std::make_shared<FluidModel<Tx, Tad>>(config->freestream->p_inf, config->freestream->rho_inf,
															config->freestream->T_inf, config->freestream->mu_inf,
															config->freestream->pr_inf);

		boundary_container = std::make_unique<BoundaryContainer<Tx, Tad>>(config->filename, mesh, config, fluid_model);
	};

	~EulerEquation(){
	};
};
template <class Tx, class Tad>
void EulerEquation<Tx, Tad>::calc_convective_residual(Array3D<Tad>& a_rhs){
	convective_flux->evaluate(mesh->normal_chi,
							  rholft_chi, ulft_chi, vlft_chi, plft_chi,
							  rhorht_chi, urht_chi, vrht_chi, prht_chi,
							  flux_chi);
	convective_flux->evaluate(mesh->normal_eta,
							  rholft_eta, ulft_eta, vlft_eta, plft_eta,
							  rhorht_eta, urht_eta, vrht_eta, prht_eta,
							  flux_eta);
#pragma omp parallel for
	for(uint i=0; i< nic; i++){
		for(uint j=0; j< njc; j++){
			for(uint k=0; k<mesh->solution->nq; k++){
				a_rhs[i][j][k] -= (flux_eta[i][j+1][k] - flux_eta[i][j][k]);
				a_rhs[i][j][k] -= (flux_chi[i+1][j][k] - flux_chi[i][j][k]);
			}
		}
	}
}

template <class Tx, class Tad>
void EulerEquation<Tx, Tad>::calc_primvars(Array3D<Tad>& a_q){
	fluid_model->primvars(a_q, rho, u, v, p, T, 1U, 1U);
}


template <class Tx, class Tad>
void EulerEquation<Tx, Tad>::calc_boundary(){
	boundary_container->apply(rho, u, v, p, T);
}

template <class Tx, class Tad>
void EulerEquation<Tx, Tad>::calc_intermediates(Array3D<Tad>& a_q){
	calc_primvars(a_q);
	calc_boundary();
	
	reconstruction->evaluate_chi(rho, rholft_chi, rhorht_chi);
	reconstruction->evaluate_chi(u, ulft_chi, urht_chi);
	reconstruction->evaluate_chi(v, vlft_chi, vrht_chi);
	reconstruction->evaluate_chi(p, plft_chi, prht_chi);

	reconstruction->evaluate_eta(rho, rholft_eta, rhorht_eta);
	reconstruction->evaluate_eta(u, ulft_eta, urht_eta);
	reconstruction->evaluate_eta(v, vlft_eta, vrht_eta);
	reconstruction->evaluate_eta(p, plft_eta, prht_eta);

	if(config->freestream->if_viscous){
		for(uint i=0; i<nic+2; i++){
			for(uint j=0; j<njc+2; j++){
				mu[i][j] = fluid_model->get_laminar_viscosity(T[i][j]);
				k[i][j] = fluid_model->get_thermal_conductivity(T[i][j]);
			}
		}
		
		mesh->calc_gradient(u, grad_u_chi, grad_u_eta);
		mesh->calc_gradient(v, grad_v_chi, grad_v_eta);
		mesh->calc_gradient(T, grad_T_chi, grad_T_eta);
		mesh->calc_face(u, u_bar_chi, u_bar_eta);
		mesh->calc_face(v, v_bar_chi, v_bar_eta);
		mesh->calc_face(T, T_bar_chi, T_bar_eta);
		mesh->calc_face(mu, mu_bar_chi, mu_bar_eta);
		mesh->calc_face(k, k_bar_chi, k_bar_eta);
	}
};

template <class Tx, class Tad>
void EulerEquation<Tx, Tad>::calc_residual(Array3D<Tad>& a_q, Array3D<Tad>& a_rhs){
	a_rhs.fill(0.0);

	calc_intermediates(a_q);

	calc_convective_residual(a_rhs);

	if(config->freestream->if_viscous){
		calc_viscous_residual(a_rhs);
	}

	calc_source_residual(a_q, a_rhs);

	// divide by volume
#pragma omp parallel for
	for(uint i=0; i< nic; i++){
		for(uint j=0; j< njc; j++){
			for(uint k=0; k<mesh->solution->nq+mesh->solution->ntrans; k++){
				a_rhs[i][j][k] /= mesh->volume[i][j];
			}
		}
	}

};

#endif
